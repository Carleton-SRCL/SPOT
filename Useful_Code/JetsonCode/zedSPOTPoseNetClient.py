
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
import math

#
#   CONSTANTS
#
#max delay of 1 single camera frame
max_delay_ns = (1/30.0)/(1e-09)
net_image_size = 448
x_median = 0.96756636
y_median = -0.01050488
yaw_median = 0
path_to_weights_const = './Experiment_2020-11-05-12:31.h5'

def saveImageAndData(name, image, data):
    photo_name = name + '.jpg'
    file_name = name + '.txt'
    cv2.imwrite(photo_name, image)
    data_file = open(file_name, 'w')
    data_file.write(data)
    data_file.close()
    print("Done saving image: " + photo_name)

def saveImagePosDataAndNetData(name, image, data, netData):
    photo_name = name + '.jpg'
    file_name = name + '.txt'
    net_file_name = name + '_estimated_net.txt'
    diff_file_name = name + '_diff_net.txt'
    print("IN SAVE DATA")
    cv2.imwrite(photo_name, image)
    data_file = open(file_name, 'w')
    data_file.write(data)
    data_file.close()

    net_data_file = open(net_file_name, 'w')
    net_data_file.write(str(netData[0]) + '\n')
    net_data_file.write(str(netData[1]) + '\n')
    net_data_file.write(str(netData[2]) + '\n')
    net_data_file.write(str(netData[3]) + '\n')
    net_data_file.close()
    groundTruthData = [None]*7
    groundTruthData[0] = float(data.splitlines()[-7])
    groundTruthData[1] = float(data.splitlines()[-6])
    groundTruthData[2] = float(data.splitlines()[-5])
    groundTruthData[3] = float(data.splitlines()[-4])
    groundTruthData[4] = float(data.splitlines()[-3])
    groundTruthData[5] = float(data.splitlines()[-2])
    groundTruthData[6] = float(data.splitlines()[-1])
#    print(groundTruthData)
    groundTruth = CustomImageDataGenerator.PositionalData(groundTruthData)
    gt_x, gt_y, gt_yaw = groundTruth.getRelativeBlackPoseFromRedNormalizedRotation()

    print("Ground Truth:")
    print(gt_x)
    print(gt_y)
    print(gt_yaw)
    print("Network Output:")
    print(netData[0])
    print(netData[1])
    print(netData[2])
    print(netData[3])

    diff_x = abs(gt_x - netData[0])
    diff_y = abs(gt_y - netData[1])
    diff_yaw = abs(gt_yaw - netData[2])

    print("Differences")
    print(diff_x)
    print(diff_y)
    print(diff_yaw)

    #diff_file = open(diff_file_name, 'w')
    #diff_file.write(gt_x)
    #diff_file.write(gt_y)
    #diff_file.write(gt_yaw)
    #diff_file.write(diff_x)
    #diff_file.write(diff_y)
    #diff_file.write(diff_yaw)
    #diff_file.close()

    print("Done Processing and Saving Data: " + photo_name)

def loadSPOTPoseNet(path_to_weights):
    print("Load SPOTPoseNet")
    print(tf.__version__)
    keras.backend.clear_session()

    print("loading with leaky relu")
    model = keras.models.load_model(path_to_weights, compile=True,  custom_objects={'LeakyReLU': tf.keras.layers.LeakyReLU,
                                                                     'custom_loss': SPOTPoseNet.custom_loss,
                                                                     'custom_metric': SPOTPoseNet.custom_metric})
    print("Loaded")
    model.summary()

    return model


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
#                print("FAILED. Sleep briefly & try again in 1 seconds")
#                time.sleep(1)
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
#                connected = False
                break

            #print('Got message: ' + str(data.decode("utf-8")))
            commQueue.put(data.decode("utf-8"))

            try:
                out_data = outComm.get(timeout=0.001)
            except queue.Empty:
                continue

            client_socket.send(out_data.encode())

    print('commsOver')

def zedCameraRecordVideoProcess(commQueue):
    zedCamera = recordStereo.StereoCamera(1, 'MJPG', 30.0, 2560, 720)
    data = None
    print("Started Camera Process")
    while True:
        try:
            data = commQueue.get(timeout=0.001)
        except queue.Empty:
            continue
        if data == 'zed stop':
            zedCamera.stopRecording()
        elif data == 'zed init':
            zedCamera.initVideo()
        elif data == 'zed start':
            zedCamera.startRecording()
        elif data == 'zed end':
            break
        else:
            continue

def zedCameraProcess(commQueue, outQueue):
    zedCamera = recordStereo.StereoCamera(1, 'MJPG', 30.0, 2560, 720)
    data = None
    image = np.zeros(shape=(720, 2560))
    last_timestamp = 0.0
    experiment_started = False
    data_requested = False
    file_name = 'preliminary_test_1'
    positional_data = ''
    data_request_time_ns = perf_counter()
    left = np.zeros(shape=(1, net_image_size, net_image_size, 1), dtype='uint8')
    right = np.zeros(shape=(1, net_image_size, net_image_size, 1), dtype='uint8')
    previousTimeStep  = ''
    zedCamera.initImageRecord()
    print("Started Camera Process")

    pose_net = loadSPOTPoseNet(path_to_weights_const)

    while True:
        try:
 #           zedTimeStart = time.time()
            data = commQueue.get(timeout=0.001)
            dataString = data[:]
            timeStep = float(dataString.splitlines()[0])
#            if not experiment_started:
#                experiment_started = True
#                print("Experiment Started")
            
#            if timeStep == previousTimeStep:
#                positional_data = ''
#                continue

            previousTimeStep = timeStep
            positional_data = ''
            positional_data = data[:]
 #           print("Data:")
 #           print(str(data[:]))

            #zedTimeStart = time.time()

            image, timestamp = zedCamera.getImageAndTimeStamp()
#            zedStop = time.time()

#            print("Zed Camera Capture Time:" + str(zedStop - zedTimeStart))
            timestamp_diff = timestamp - last_timestamp
            print("Time Stamp Difference")
            print(timestamp_diff)
            # new frame to match if diff > 10ms
            if timestamp_diff > 10.0 and not positional_data == '':
                last_timestamp = timestamp
                data_request_time_ns = perf_counter()/(1e-09)
                file_name = './CameraData/' + datetime.datetime.today().strftime('%Y-%m-%d-%H-%M-%S-%f')
                # send request to PI for positional data
                out_data = str(1) + "\n"
                #outQueue.put(out_data)
                #start = time.time()
                image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
                stereoFrame = np.split(image, 2, axis=1)

                left[:, :, :, 0] = cv2.resize(stereoFrame[0], (net_image_size, net_image_size))
                right[:, :, :, 0] = cv2.resize(stereoFrame[1], (net_image_size, net_image_size))

                stacked_stereo = np.concatenate((left, right), axis=3)
  #              start = time.time()
                output = pose_net(stacked_stereo, training=False)
   #             end = time.time()

                #print("Inference time: " + str(end-start))
                x = float(output[0]) + x_median
                y = float(output[1]) + y_median
                yaw = ((float(output[2]) + yaw_median) * 2 * math.pi) - math.pi

                netData = [x, y, yaw, float(output[3])]
                saveImagePosDataAndNetData(file_name, image, data, netData)

                #saveImageAndData(file_name, image, positional_data)
                #reset data and request
                #outQueue.put(out_data)
#                end = time.time()
                positional_data = ''
#                print("Inference time + Saving: " + str(end-start))
#while not commQueue.empty():
#                    commQueue.get()

        except queue.Empty:
#            experiment_started = False
#            print("Experiment Stopped")
            continue

        while not commQueue.empty():
            commQueue.get()


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


