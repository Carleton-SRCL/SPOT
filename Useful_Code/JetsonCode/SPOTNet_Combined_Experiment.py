import numpy as np
import cv2
from tensorflow import keras
import tensorflow as tf
import sys
import time
import SPOTPoseNet
import os
import CustomVideoDataGenerator
import CustomImageDataGenerator
import matplotlib.pyplot as plt
import math
from datetime import datetime
import socket
import recordStereo 

outputYawSize = 128
image_size = 448
stop_count = 500
x_median = 1.46221606
y_median = -0.05725426


##################################################
        #### Load SPOTNet Weights  #####
##################################################

print(tf.__version__)
keras.backend.clear_session()
path_to_weights = sys.argv[1]

print("loading with leaky relu")
model = keras.models.load_model(path_to_weights,  custom_objects={'LeakyReLU': tf.keras.layers.LeakyReLU,
                                                                     'custom_loss': SPOTPoseNet.custom_loss,
                                                                     'custom_metric': SPOTPoseNet.custom_metric})
model.summary()
zedCamera = recordStereo.StereoCamera(1, 'MJPG', 30.0, 2560, 720)
zedCamera.initImageRecord()
left = np.zeros(shape=(1, image_size, image_size, 1))
right = np.zeros(shape=(1, image_size, image_size, 1))

##################################################
#### Start communication with JetsonRepeater #####
##################################################

# Initializing
connected = False
#image, timestamp = zedCamera.initVideo()

while True:
#     frame, timestamp = zedCamera.getImageAndTimeStamp()
#     frame, timestamp = zedCamera.getImageAndTimeStamp()
#    frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    if not connected: #try to connect to the JetsonRepeater program
        try:  # Try to connect
            client_socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            client_socket.connect("/tmp/jetsonRepeater")  # Connecting...
            connected = True
            print("Connected to JetsonRepeater! Deep Guidance Started")
        except:  # If connection attempt failed
            print("Connection to JetsonRepeater FAILED. Trying to re-connect in 1 second")
            time.sleep(1)
            continue  # Restart from while True
    else:  # We are connected!

        ##################################################
                    #### Run SPOTNet  #####
        ##################################################
        start = time.time()

        image, timestamp = zedCamera.getImageAndTimeStamp()

        if len(image.shape) == 2:
            continue

#        try:  # Try to receive data
#            out_data = "CameraShutter\n"
#            client_socket.send(out_data.encode())
#        except:  # If receive fails, we have lost communication with the JetsonRepeater
#            print("Lost communication with JetsonRepeater")
#            connected = False

        image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        stereoFrame = np.split(image, 2, axis=1)

        left[:, :, :, 0] = cv2.resize(stereoFrame[0], (image_size, image_size))
        right[:, :, :, 0] = cv2.resize(stereoFrame[1], (image_size, image_size))

        stacked_stereo = np.concatenate((left, right), axis=3)

        output = model.predict(stacked_stereo)
        end = time.time()

        maxIdxVal = 0
        maxIdx = 0
        outputIndexs = output[2]
#        print(outputIndexs)
        for yawIdx in range(0, outputYawSize):
            if float(outputIndexs[0][yawIdx]) > maxIdxVal:
                maxIdxVal = float(outputIndexs[0][yawIdx])
                maxIdx = yawIdx

        yaw = ((maxIdx + 1) / outputYawSize) * 2 * math.pi - math.pi
        x = float(output[0] + x_median)
        y = float(output[1] + y_median)

        print("Network Output")
        print("X: " + str(abs(x)) + " Y: " + str(y) + " Yaw: " + str(yaw) + " Confidence: " + str(float(output[3])))
        print("\nInference Speed: ", str(end-start))

        try:  # Try to receive data
            out_data = "SPOTNet\n" + str(x) + "\n" + str(y) + "\n" + str(yaw) + "\n" + str(float(output[3])) + "\n"
            client_socket.send(out_data.encode())
        except:  # If receive fails, we have lost communication with the JetsonRepeater
            print("Lost communication with JetsonRepeater")
            connected = False
            continue  # Restart from while True

print("\n\nCompleted\n\n")
