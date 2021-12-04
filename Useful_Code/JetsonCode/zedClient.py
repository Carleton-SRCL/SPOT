
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
                #print("FAILED. Sleep briefly & try again in 1 seconds")
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

            #print('Got message: ' + str(data.decode("utf-8")))
            commQueue.put(data.decode("utf-8"))

            try:
                out_data = outComm.get(timeout=0.001)
            except queue.Empty:
                continue

            client_socket.send(out_data.encode())

    print('commsOver')

def zedCameraRecordVideoProcess(commQueue):
    zedCamera = recordStereo.StereoCamera(1, 'MJPG', 15.0, 2560, 720)
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
    file_name = ''
    positional_data = ''
    data_request_time_ns = perf_counter()

    previousTimeStep  = ''
    zedCamera.initImageRecord()
    print("Started Camera Process")

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
            print(timestamp)
            print(last_timestamp)
            # new frame to match if diff > 10ms
            if timestamp_diff > 10.0 and not positional_data == '':
                last_timestamp = timestamp
                data_request_time_ns = perf_counter()/(1e-09)
                file_name = '/home/spot/Frank/spot/CameraData/' + datetime.datetime.today().strftime('%Y-%m-%d-%H-%M-%S-%f')
                # send request to PI for positional data
                out_data = str(1) + "\n"
                outQueue.put(out_data)

                saveImageAndData(file_name, image, positional_data)
                #reset data and request
                positional_data = ''

        except queue.Empty:
            #experiment_started = False
            #print("Experiment Stopped")
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


