
import sys
import recordStereo
import socket
import queue
from multiprocessing import Process, Queue
from time import perf_counter
import time
import cv2
import datetime
import numpy as np
from tensorflow import keras
import tensorflow as tf
import SPOTPoseNet
import os
import CustomVideoDataGenerator
import CustomImageDataGenerator
import matplotlib.pyplot as plt
import math

experimentName = sys.argv[1]
path_to_weights = sys.argv[2]

outputYawSize = 128
image_size = int(sys.argv[3])
x_median = float(sys.argv[4]) #1.46221606
y_median = float(sys.argv[5]) # -0.05725426

#max delay of 1 single camera frame
max_delay_ns = (1/30.0)/(1e-09)

def saveImageAndData(name, image, data):
    photo_name = name + '.jpg'
    file_name = name + '.txt'
    cv2.imwrite(photo_name, image)
    data_file = open(file_name, 'w')
    data_file.write(data)
    data_file.close()
    print("Done saving image: " + photo_name)

#threading function for data transfer
def communicationsProcess(commQueue, outComm):
    connected = False
    while True:
        if not connected:
            try:
                client_socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
                client_socket.connect("/tmp/jetsonRepeater")
                connected = True
                print("Connected to Server! Waiting commands")
            except:
                print("FAILED. Sleep briefly & try again in 1 seconds")
                #time.sleep(1)
                continue
        else:
            try:
                data = client_socket.recv(5120)
            except:
                print("Lost communications")
                connected = False
                continue
            if not data:
                commQueue.put('zed end')
                print('ENDED')
                break

            while not commQueue.empty():
                try:
                    commQueue.get(False)
                except:
                    print("Its empty")
                    break

            #print('Got message: ' + str(data.decode("utf-8")))
            commQueue.put(data.decode("utf-8"))
            time.sleep(0.005)
            #try:
            #    out_data = outComm.get(timeout=0.001)
            #except queue.Empty:
            #    continue

            #client_socket.send(out_data.encode())

    print('commsOver')

def zedCameraProcess(commQueue, outQueue):
##################################################
        #### Load SPOTNet Weights  #####
##################################################

    print(tf.__version__)
    keras.backend.clear_session()
#    path_to_weights = sys.argv[1]

    print("loading with leaky relu")
    model = keras.models.load_model(path_to_weights,  custom_objects={'LeakyReLU': tf.keras.layers.LeakyReLU,
                                                                     'custom_loss': SPOTPoseNet.custom_loss,
                                                                     'custom_metric': SPOTPoseNet.custom_metric})
    model.summary()
    zedCamera = recordStereo.StereoCamera(1, 'MJPG', 30.0, 2560, 720)
#    zedCamera.initImageRecord()
    left = np.zeros(shape=(1, image_size, image_size, 1))
    right = np.zeros(shape=(1, image_size, image_size, 1))
    #zedCamera = recordStereo.StereoCamera(1, 'MJPG', 30.0, 2560, 720)
    data = None
    image = np.zeros(shape=(720, 2560))
    last_timestamp = 0.0
    experiment_started = False
    data_requested = False
    file_name = ''
    positional_data = ''
    data_request_time_ns = perf_counter()

    previousTimeStep  = ''
    zedCamera.initImageRecord()
    print("Started Camera Process")

    image, timestamp = zedCamera.getImageAndTimeStamp()
    while len(image.shape) == 2:
        image, timestamp = zedCamera.getImageAndTimeStamp()

    image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    stereoFrame = np.split(image, 2, axis=1)

    left[:, :, :, 0] = cv2.resize(stereoFrame[0], (image_size, image_size))
    right[:, :, :, 0] = cv2.resize(stereoFrame[1], (image_size, image_size))

    stacked_stereo = np.concatenate((left, right), axis=3)

    output = model.predict(stacked_stereo)
    while True:
        try:
            data = commQueue.get(timeout=0.001)
            dataString = data[:]
            timeStep = float(dataString.splitlines()[0])
            if not experiment_started:
                experiment_started = True
                print("Experiment Started")
            
            if timeStep == previousTimeStep:
                positional_data = ''
                continue

            previousTimeStep = timeStep
            positional_data = ''
            positional_data = data[:]
            #print("Data:")
            #print(str(data[:]))

            image, timestamp = zedCamera.getImageAndTimeStamp()
            timestamp_diff = timestamp - last_timestamp
            start = time.time()
            #print(timestamp)
            #print(last_timestamp)
            # new frame to match if diff > 10ms
            if len(image.shape) == 2:
                print("Bad image")
                continue

            if True:
                #last_timestamp = timestamp
                #data_request_time_ns = perf_counter()/(1e-09)
                file_name = '/home/spot/Frank/spot/CameraData/'+ experimentName + datetime.datetime.today().strftime('%Y-%m-%d-%H-%M-%S-%f')
                # send request to PI for positional data
                #out_data = str(1) + "\n"
                #outQueue.put(out_data)
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

                timeToRun = str(end-start)
                
                positional_data = positional_data + "\nx = " + str(x) +"\ny = " + str(y) + "\nyaw = " + str(yaw) + "\nTime = " + timeToRun + "\nConfidence: " + str(float(output[3]))
                
                saveImageAndData(file_name, image, positional_data)
                #reset data and request
                positional_data = ''
        except queue.Empty:
            #experiment_started = False
#            print("No Data")
            continue

def Main():
    commQueue = Queue()
    outQueue = Queue()

    #add out coms for comsProcess
    comms = Process(target=communicationsProcess, args=(commQueue, outQueue))
    comms.start()

    camera = Process(target=zedCameraProcess, args=(commQueue, outQueue))
    camera.start()

    comms.join()
    print("Comms Done")
    camera.join()
    print("Camera Done")


if __name__ == "__main__":
    Main()


