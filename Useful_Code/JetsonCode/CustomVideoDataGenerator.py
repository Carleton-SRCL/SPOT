#import matplotlib.pyplot as plt
import numpy as np
import cv2
import sys
import h5py
import os
from time import localtime, strftime
from random import shuffle
from pathlib import Path
#import matplotlib.pyplot as p


def getVideoInformation(video):
    numFrames = 0
    frameXLength = 0
    frameYLength = 0
    try:
        cap = cv2.VideoCapture(video)
        numFrames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
        frameXLength = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        frameYLength = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        #print("Total Number of Frames: " + str(numFrames))

    except Exception as e:
        sys.exit(e)

    return numFrames, frameYLength, frameXLength, cap

def generateGrayTimeSeriesDataFromVideoBatch(videoBatch, x_size, y_size, mean, std):
    image_size_x = x_size*2
    image_size_y = y_size
    num_channels = 1

    stereo_dataset = np.zeros(shape=(len(videoBatch), image_size_y, image_size_x, num_channels), dtype='uint8')

    groundTruth = []
    frameNum = 0

    for videoPath, videoFrameIdx in videoBatch:
        cap = cv2.VideoCapture(videoPath)
        cap.set(cv2.CAP_PROP_POS_FRAMES, videoFrameIdx)

        ret, frame = cap.read()
        stereoFrame = np.split(frame, 2, axis=1)

        left = cv2.cvtColor(cv2.resize(stereoFrame[0], (y_size, x_size)), cv2.COLOR_BGR2GRAY)
        right = cv2.cvtColor(cv2.resize(stereoFrame[1], (y_size, x_size)), cv2.COLOR_BGR2GRAY)

        stereo_dataset[frameNum, :, :, 0] = np.concatenate((left, right), axis=1)

        labelPath = videoPath[:-4] + ".h5"

        groundTruth.append((getGroundTruth(labelPath, videoFrameIdx) - mean)/std)

        frameNum += 1

    return stereo_dataset, groundTruth

def normalizeData(data):
    mean = data.mean(axis=0)
    std = data.std(axis=0)

    data -= mean
    data /= std

    return data, mean, std

def generateGrayTimeSeriesDataFromVideoSegment(videoPath, start, stop, pathToLabels):
    image_size_x = 1280
    image_size_y = 720
    num_channels = 1

    numFrames = stop - start
    startFrame = start
    cap = cv2.VideoCapture(videoPath)
    cap.set(cv2.CAP_PROP_POS_FRAMES, startFrame)

    datasetLeft = np.zeros(shape=(numFrames, image_size_y, image_size_x, num_channels), dtype='uint8')
    datasetRight = np.zeros(shape=(numFrames, image_size_y, image_size_x, num_channels), dtype='uint8')

    frameCount = 0

    ret = True
    while (frameCount < numFrames and ret):
        ret, frame = cap.read()
        stereo_image_split = np.split(frame, 2, axis=1)

        datasetLeft[:, :, 0] = cv2.cvtColor(cv2.resize(stereo_image_split[0], (image_size_y, image_size_x)), cv2.COLOR_BGR2GRAY)
        datasetRight[:, :, 0] = cv2.cvtColor(cv2.resize(stereo_image_split[1], (image_size_y, image_size_x)),cv2.COLOR_BGR2GRAY)
        frameCount += 1

    cap.release()

    groundTruth = generateTrainingLabelsSegment(size=numFrames, start=start, stop=stop, pathToLabels=pathToLabels)

    stereo = np.append(datasetLeft, datasetRight, axis=1)

    return stereo, groundTruth

def generateTrainingLabelsSegment(size, start, stop, pathToLabels):
    with h5py.File(pathToLabels, 'r') as f:
        #print("file found: " + pathToLabels) 
        data = f['/dataProcessedOut'][()]

    data = data.transpose()

    groundTruth = data[start:stop]

    return groundTruth

def getGroundTruth(pathToLabel, frameIdx):
    with h5py.File(pathToLabel, 'r') as f:
        data = f['/dataProcessedOut'][()]

    data = data.transpose()
    groundTruth = data[frameIdx]

    return groundTruth

def getTrainingLabelInfo(pathToLabels):
    with h5py.File(pathToLabels, 'r') as f:
        data = f['/dataProcessedOut'][()]
    num_rows, num_cols = data.shape
    return num_cols if num_cols > num_rows else num_rows

def searchFilesInDirecotry(path, key='**/'):
    filePaths = []
    for filename in Path(path).glob(key):
        head, tail = os.path.split(filename)
        if tail not in filePaths:
            filePaths.append(str(filename))
    return filePaths

def datasetTrainValidationSplit(path, training_split, file_extension, saveToFile=False):
    filePaths = searchFilesInDirecotry(path, file_extension)

    if training_split > 1:
        training_split = 1

    if training_split <= 0.1:
        training_split = 0.1

    # pop self path reference
    shuffle(filePaths)

    trainingSize = int(len(filePaths) * training_split)
    validationSize = len(filePaths) - trainingSize

    trainingSet = filePaths[0:trainingSize]
    validationSet = filePaths[trainingSize:trainingSize + validationSize]

    print("Training Set total ", len(trainingSet), " :")
    #print(trainingSet)

    print("Validation Set total ", len(validationSet), " :")
    #print(validationSet)

    # check to make sure no cross contamination
    assert (not set(trainingSet) & set(validationSet)), "Cross over between Training set and Validation set!"

    currentTime = strftime("%Y-%m-%d-%H-%M", localtime())
    saveDir = "Training_" + currentTime + "/"

    if saveToFile:
        try:
            os.makedirs(saveDir)
        except FileExistsError:
            # directory already exists
            pass

        file = saveDir + "Train.txt"
        print(file)
        with open(file, 'w') as f:
            for item in trainingSet:
                f.write("%s\n" % item)
        file = saveDir + "Validation.txt"
        with open(file, 'w') as f:
            for item in validationSet:
                f.write("%s\n" % item)

    return trainingSet, validationSet, saveDir

def getDataset(path, file_extension='**/*.avi'):
    filePaths = searchFilesInDirecotry(path, file_extension)

    # pop self path reference
    shuffle(filePaths)

    datasetSize = int(len(filePaths))

    dataset = filePaths[0:datasetSize]

    print("Dataset total ", datasetSize, " :")
    #print(dataset)

    return dataset

def datasetShuffleAndSplit(dataset, train_ratio):
    shuffled_dataset = datasetShuffle(dataset)

    trainingSize = int(len(shuffled_dataset) * train_ratio)
    validationSize = len(shuffled_dataset) - trainingSize

    trainingSet = shuffled_dataset[0:trainingSize]
    validationSet = shuffled_dataset[trainingSize:trainingSize + validationSize]

    print("Training Set total ", len(trainingSet), " :")
    #print(trainingSet)

    print("Validation Set total ", len(validationSet), " :")
    #print(validationSet)

    # check to make sure no cross contamination
    assert (not set(trainingSet) & set(validationSet)), "Cross over between Training set and Validation set!"

    return trainingSet, validationSet

#shuffling start positions for the data
def datasetShuffle(datasetPaths):
    videoPath = []
    videoFrameIdx = []

    for datasetPath in datasetPaths:
        cap = cv2.VideoCapture(datasetPath)
        totalNumFrames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
        cap.release()
        labelPath = datasetPath[:-3] + 'h5'
        max_num_labels = getTrainingLabelInfo(labelPath)

        if totalNumFrames > max_num_labels:
            totalNumFrames = max_num_labels

        for num in range(int(totalNumFrames)):
            videoFrameIdx.append(num)
            videoPath.append(datasetPath)

    tupleList = list(zip(videoPath, videoFrameIdx))
    shuffle(tupleList)

    return tupleList

def loadExistingExperiment(path):

    trainingFilename = path + "Train.txt"
    validationFilename = path + "Validation.txt"
    testingFilename = path + "Test.txt"

    with open(trainingFilename) as f:
        trainingFileData = f.readlines()
    # you may also want to remove whitespace characters like `\n` at the end of each line
    trainingSet = [x.strip() for x in trainingFileData]

    with open(validationFilename) as f:
        validationFileData = f.readlines()
        # you may also want to remove whitespace characters like `\n` at the end of each line
    validationSet = [x.strip() for x in validationFileData]

    with open(testingFilename) as f:
        testFileData = f.readlines()
        # you may also want to remove whitespace characters like `\n` at the end of each line
    testSet = [x.strip() for x in testFileData]

    return trainingSet, validationSet, testSet


def Main():
    train, validation, test = loadExistingExperiment(sys.argv[1])
    print("Train")
    print(train)

    print("Validation")
    print(validation)

    print("Test")
    print(test)

    #generateAndSaveTimeSeriesData(sys.argv[1])
    # leftData, rightData = generateGrayTimeSeriesDataFromVideo(sys.argv[1])
    # dataSize = leftData.shape
    # print(dataSize[0])
    # trainingData = generateTrainingLabels(dataSize[0])

if __name__ == "__main__":
    Main()
