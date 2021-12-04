import datetime
import cv2
import math
import os
import threading
from collections import deque
import numpy as np
import time
from time import sleep

class StereoCamera:
    def __init__(self, _camera, _format, _fps, _x_res, _y_res):
        directory = os.path.expanduser("~/Desktop/Videos")
        if not os.path.exists(directory):
            os.makedirs(directory)
        self.__camera = _camera
        self.__format = _format
        self.__fps = _fps
        self.__x_res = _x_res
        self.__y_res = _y_res
        self.__cap = cv2.VideoCapture()  # this is dependent on number of cameras present...
        self.__fourcc = cv2.VideoWriter_fourcc(*self.__format)
        self.__out = cv2.VideoWriter()#('{date:%Y-%m-%d_%H_%M_%S}.avi'.format(date=datetime.datetime.now()), self.__fourcc,
                                  # self.__fps, (self.__x_res, self.__y_res))
        self.__initialized = False
        self.__frame_queue = deque()
        self.__lock = threading.Lock()
        self.__recording = False
        self.__recordVideoThread = threading.Thread()
        self.__saveVideoThread = threading.Thread()
        self.__droppedFrameCheck = 0.001
        self.__timeStamp = ""
        self.__droppedFrameList = []
        self.__image_frame = np.zeros(shape=(_y_res, _x_res))
        self.__image_timestamp = 0.0
        self.__last_image_timestamp = 0.0
        self.__stop_recording_images = False

    @property
    def format(self):
        return self.__format

    @property
    def fps(self):
        return self.__cap.get(cv2.CAP_PROP_FPS)

    @property
    def x_resolution(self):
        return self.__x_res

    @property
    def y_resolution(self):
        return self.__y_res

    @format.setter
    def format(self, format_type):
        self.__format = format_type
        self.__fourcc = cv2.VideoWriter_fourcc(*self.__format)

    @fps.setter
    def fps(self, fps):
        self.__fps = fps
        self.__cap.set(cv2.CAP_PROP_FPS, fps)

    @x_resolution.setter
    def x_resolution(self, x_res):
        self.__x_res = x_res

    @y_resolution.setter
    def y_resolution(self, y_res):
        self.__y_res = y_res

    def isInitialized(self):
        return self.__initialized

    def startRecording(self):
        print("Starting to Record")
        self.__lock.acquire()
        if self.__initialized:
            self.__recording = True
        else:
            print("Not initialized")
        self.__lock.release()
        return

    def stopRecording(self):
        self.__lock.acquire()
        self.__initialized = False
        self.__recording = False
        self.__lock.release()

        print("Number of threads active: ", threading.active_count())

        if self.__cap.isOpened():
            self.__cap.release()
        if self.__out.isOpened():
            self.__out.release()

        if self.__recordVideoThread.isAlive():
            print("Waiting for recordVideo to be joined")
            self.__recordVideoThread.join()
        print("Number of threads active: ", threading.active_count())

        if self.__saveVideoThread.isAlive():
            print("Waiting for save video to be joined")
            self.__saveVideoThread.join()
        print("Number of threads active: ", threading.active_count())

        print("Total list of dumped frames")
        with open(self.__timeStamp + ".txt", "w") as file:
            for droppedFrames in self.__droppedFrameList:
                file.write("%s\n" % droppedFrames)

        return

    def initVideo(self):
        print("ZED Initialized")
        if self.__recordVideoThread.isAlive() or self.__saveVideoThread.isAlive():
            print("Threads left alive")
            self.stopRecording()

        self.__lock.acquire()
        self.__cap = cv2.VideoCapture(self.__camera)  # this is dependent on number of cameras present...
        self.__cap.set(cv2.CAP_PROP_FPS, self.__fps)
        self.__cap.set(cv2.CAP_PROP_FRAME_WIDTH, self.__x_res)
        self.__cap.set(cv2.CAP_PROP_FRAME_HEIGHT, self.__y_res)
        self.__timeStamp = '/home/spot/Desktop/Videos/{date:%Y-%m-%d-%H-%M-%S}'.format(date=datetime.datetime.now())
        self.__out = cv2.VideoWriter(self.__timeStamp + ".avi",
                                     self.__fourcc,
                                     self.__fps,
                                     (int(self.__x_res), int(self.__y_res))
                                     )
        self.__droppedFrameCheck = math.ceil((1/self.__fps)*1000)
        print("Dropped Frames Detected if difference between frames are greater than " + str(self.__droppedFrameCheck) + " MSECs")
        self.__droppedFrameList = []
        self.__initialized = True
        self.__lock.release()

        self.__recordVideoThread = threading.Thread(target=self.__recordVideo, args=())
        self.__saveVideoThread = threading.Thread(target=self.__saveVideo, args=())
        self.__recordVideoThread.start()
        self.__saveVideoThread.start()


    def __recordVideo(self):
        print("started record thread")
        oldStamp = 0
        frameCounter = 0;
        while True:
            ret, frame = self.__cap.read()
            diff = self.__cap.get(cv2.CAP_PROP_POS_MSEC) - oldStamp
            if diff > self.__droppedFrameCheck and frameCounter > 0:
                print("Dropped Frame between frames: " + str(frameCounter) + " & "
                      + str(frameCounter - 1) + "With a difference of " + str(diff))
                out = str(frameCounter - 1) + "," + str(frameCounter) + "," + str(int(round(diff/((1/self.__fps)*1000)))) + "," + str(diff)
                self.__droppedFrameList.append(out)
            oldStamp = self.__cap.get(cv2.CAP_PROP_POS_MSEC)
            self.__lock.acquire()
            if self.__recording:
                self.__frame_queue.append(frame)
                frameCounter = frameCounter + 1
            if not self.__initialized:
                self.__lock.release()
                break
            self.__lock.release()
        print("Done recording")

    def __saveVideo(self):
        print("started save thread")
        while True:
            self.__lock.acquire()
            if not self.__frame_queue:
                if not self.__recording and not self.__initialized and not self.__recordVideoThread.isAlive():
                    self.__lock.release()
                    print("Empty and no Recording")
                    break
            else:
                self.__out.write(self.__frame_queue.popleft())

            self.__lock.release()
        print("Done saving video")

    def initImageRecord(self):
        print("ZED Initialized")

        self.__lock.acquire()
        self.__cap = cv2.VideoCapture(self.__camera)  # this is dependent on number of cameras present...
        self.__cap.set(cv2.CAP_PROP_FPS, self.__fps)
        self.__cap.set(cv2.CAP_PROP_FRAME_WIDTH, self.__x_res)
        self.__cap.set(cv2.CAP_PROP_FRAME_HEIGHT, self.__y_res)
        self.__lock.release()

        self.__recordVideoThread = threading.Thread(target=self.__recordImage, args=())
        self.__recordVideoThread.start()

    def stopImageRecord(self):
        self.__stop_recording_images = True

    def __recordImage(self):
        print("started record thread")
        while True:
            ret, frame = self.__cap.read()
            self.__lock.acquire()
            self.__image_timestamp = self.__cap.get(cv2.CAP_PROP_POS_MSEC)
            self.__image_frame = frame
            stop = self.__stop_recording_images
            self.__lock.release()
            sleep(0.001)
            if stop:
                break
        print("Done recording")

    def getImageAndTimeStamp(self):
        self.__lock.acquire()
        print("getImageAndTimeStamp")
        timestamp = self.__image_timestamp
        image = self.__image_frame
        self.__lock.release()

        return image, timestamp

    def saveImageAndData(self, name, data):
        timer = time.clock()
        photo_name = name + '.jpg'
        file_name = name + '.txt'

        self.__lock.acquire()
        image = self.__image_frame
        self.__lock.release()

        print("Start saving")
        cv2.imwrite(photo_name, image)
        data_file = open(file_name, 'w')
        data_file.write(data)
        data_file.close()
        self.__last_image_timestamp = last
        print("Done saving image: " + photo_name)
        timer = time.clock() - timer
        print("Time to save Image: " + str(timer))





