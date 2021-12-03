#import matplotlib.pyplot as plt
import numpy as np
import cv2
import sys
import h5py
import shutil
import os
from time import localtime, strftime
from random import shuffle
from pathlib import Path
import math
#import matplotlib.pyplot as p


class PositionalData():
    # def __init__(self, time, r_x, r_y, r_rz, b_x, b_y, b_rz):
    #     self.timestamp = time
    #     self.red_x_pos = float(r_x)
    #     self.red_y_pos = float(r_y)
    #     self.red_z_rot = float(r_rz)
    #     self.black_x_pos = float(b_x)
    #     self.black_y_pos = float(b_y)
    #     self.black_z_rot = float(b_rz)

    def __init__(self, data):
        self.timestamp = data[0]
        self.red_x_pos = float(data[1])
        self.red_y_pos = float(data[2])
        self.red_z_rot = float(data[3])
        self.black_x_pos = float(data[4])
        self.black_y_pos = float(data[5])
        self.black_z_rot = float(data[6])

    def getRelativeBlackPoseFromRed(self):
        # T_w_b transformation matrix from black to world
        cos_b = math.cos(self.black_z_rot)
        sin_b = math.sin(self.black_z_rot)


        T_w_b = np.array([[cos_b, -sin_b, self.black_x_pos],
                          [sin_b, cos_b, self.black_y_pos],
                          [0, 0, 1]])


        # T_w_r transformation matrix from red to world
        cos_r = math.cos(self.red_z_rot)
        sin_r = math.sin(self.red_z_rot)

        rot_w_r = np.array([[cos_r, -sin_r],
                            [sin_r, cos_r]])

        t_w_r = np.array([[self.red_x_pos],
                          [self.red_y_pos]])

        rot_r_w = np.array(np.linalg.inv(rot_w_r))

        t_r_w = -1*rot_r_w.dot(t_w_r)

        T_r_w = np.array([[rot_r_w[0][0], rot_r_w[0][1], t_r_w[0][0]],
                          [rot_r_w[1][0], rot_r_w[1][1], t_r_w[1][0]],
                          [0, 0, 1]])

        T_r_b = T_r_w.dot(T_w_b)

        rel_x_pose = round(T_r_b[0][2], 4)
        rel_y_pose = round(T_r_b[1][2], 4)
        rel_z_rot = round(math.atan2(T_r_b[1][0], T_r_b[0][0]), 4)

        return rel_x_pose, rel_y_pose, rel_z_rot


    def getRelativeBlackPoseFromRedNormalizedRotation(self):
        # T_w_b transformation matrix from black to world
        cos_b = math.cos(self.black_z_rot)
        sin_b = math.sin(self.black_z_rot)


        T_w_b = np.array([[cos_b, -sin_b, self.black_x_pos],
                          [sin_b, cos_b, self.black_y_pos],
                          [0, 0, 1]])


        # T_w_r transformation matrix from red to world
        cos_r = math.cos(self.red_z_rot)
        sin_r = math.sin(self.red_z_rot)

        rot_w_r = np.array([[cos_r, -sin_r],
                            [sin_r, cos_r]])

        t_w_r = np.array([[self.red_x_pos],
                          [self.red_y_pos]])

        rot_r_w = np.array(np.linalg.inv(rot_w_r))

        t_r_w = -1*rot_r_w.dot(t_w_r)

        T_r_w = np.array([[rot_r_w[0][0], rot_r_w[0][1], t_r_w[0][0]],
                          [rot_r_w[1][0], rot_r_w[1][1], t_r_w[1][0]],
                          [0, 0, 1]])

        T_r_b = T_r_w.dot(T_w_b)

        rel_x_pose = round(T_r_b[0][2], 4)
        rel_y_pose = round(T_r_b[1][2], 4)
        rel_z_rot = round(math.atan2(T_r_b[1][0], T_r_b[0][0]), 4)

        rel_z_rot += math.pi

        rel_z_rot /= 2*math.pi

        return rel_x_pose, rel_y_pose, rel_z_rot

def normalizeData(data):
    mean = data.mean(axis=0)
    #std = data.std(axis=0)
    std = np.ones(2)
    data -= mean
    data /= std

    return data, mean, std


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
        # print(file)
        with open(file, 'w') as f:
            for item in trainingSet:
                f.write("%s\n" % item)
        file = saveDir + "Validation.txt"
        with open(file, 'w') as f:
            for item in validationSet:
                f.write("%s\n" % item)

    return trainingSet, validationSet, saveDir


def getPathsToImages(path, file_extension):
    filePaths = searchFilesInDirecotry(path, file_extension)

    # pop self path reference
    shuffle(filePaths)

    datasetSize = int(len(filePaths))

    dataset = filePaths[0:datasetSize]

    print("Dataset total ", datasetSize, " :")
    #print(dataset)

    return dataset

def getDataset(path):
    file_extension_to_good_data = 'good_data/**/*.jpg'
    file_extension_to_null_data = 'null_data/**/*.jpg'
    goodDataFilePaths = searchFilesInDirecotry(path, file_extension_to_good_data)
    nullDataFilePaths = searchFilesInDirecotry(path, file_extension_to_null_data)

    # pop self path reference
    shuffle(goodDataFilePaths)

    shuffle(nullDataFilePaths)

    goodDatasetSize = int(len(goodDataFilePaths))
    nullDatasetSize = int(len(nullDataFilePaths))

    goodDataset = goodDataFilePaths[0:goodDatasetSize]
    nullDataset = nullDataFilePaths[0:nullDatasetSize]

    print("Good Dataset total ", goodDatasetSize, " :")
    print("Null Dataset total ", nullDatasetSize, " :")

    #print(dataset)

    return goodDataset, nullDataset

def datasetShuffleAndSplit(goodDataset, nullDataset, train_ratio):
    goodTrainingSize = int(len(goodDataset) * train_ratio)
    goodValidationSize = len(goodDataset) - goodTrainingSize

    nullTrainingSize = int(len(nullDataset) * train_ratio)
    nullValidationSize = len(nullDataset) - nullTrainingSize

    trainingSet = goodDataset[0:goodTrainingSize] + nullDataset[0:nullTrainingSize]
    validationSet = goodDataset[goodTrainingSize:(goodTrainingSize + goodValidationSize)] + nullDataset[nullTrainingSize:(nullTrainingSize + nullValidationSize)]

    shuffle(trainingSet)
    shuffle(validationSet)

    print("Training Set total ", len(trainingSet), " :")
    #print(trainingSet)

    print("Validation Set total ", len(validationSet), " :")
    #print(validationSet)

    # check to make sure no cross contamination
    assert (not set(trainingSet) & set(validationSet)), "Cross over between Training set and Validation set!"

    saveDir = './savedDatSetList'

    try:
        os.makedirs(saveDir)
    except FileExistsError:
        # directory already exists
        pass

    file = saveDir + "Train.txt"
    # print(file)
    with open(file, 'w') as f:
        for item in trainingSet:
            f.write("%s\n" % item)
    file = saveDir + "Validation.txt"
    with open(file, 'w') as f:
        for item in validationSet:
            f.write("%s\n" % item)

    return trainingSet, validationSet

def generateTestDataset(goodDataset, nullDataset, train_ratio):
    goodTrainingSize = int(len(goodDataset) * train_ratio)
    goodTestSize = len(goodDataset) - goodTrainingSize

    nullTrainingSize = int(len(nullDataset) * train_ratio)
    nullTestSize = len(nullDataset) - nullTrainingSize

    trainingSet = goodDataset[0:goodTrainingSize] + nullDataset[0:nullTrainingSize]
    TestSet = goodDataset[goodTrainingSize:goodTrainingSize + goodTestSize] + nullDataset[nullTrainingSize:nullTrainingSize + nullTestSize]

    shuffle(trainingSet)
    shuffle(TestSet)

    print("Training Set total ", len(trainingSet), " :")
    #print(trainingSet)

    print("Test Set total ", len(TestSet), " :")
    #print(TestSet)

    # check to make sure no cross contamination
    assert (not set(trainingSet) & set(TestSet)), "Cross over between Training set and Test set!"

    saveDir = './savedDatSetList'

    try:
        os.makedirs(saveDir)
    except FileExistsError:
        # directory already exists
        pass

    file = saveDir + "Train.txt"
    # print(file)
    with open(file, 'w') as f:
        for item in trainingSet:
            f.write("%s\n" % item)
    file = saveDir + "Test.txt"
    with open(file, 'w') as f:
        for item in TestSet:
            f.write("%s\n" % item)

    return trainingSet, TestSet


def generateImageBatchDataAndLabels(pathToFolder, image_batch, x_size, y_size, mean, std):
    image_size_x = x_size * 2
    image_size_y = y_size
    num_channels = 1
    image_color = 0  # 0 = grayscale

    stereo_dataset = np.zeros(shape=(len(image_batch), image_size_y, image_size_x, num_channels), dtype='uint8')

    groundTruth = []
    frameNum = 0

    for image_path in image_batch:
        img = cv2.imread(image_path, image_color)

        stereoFrame = np.split(img, 2, axis=1)

        left = cv2.resize(stereoFrame[0], (y_size, x_size))
        right = cv2.resize(stereoFrame[1], (y_size, x_size))

        stereo_dataset[frameNum, :, :, 0] = np.concatenate((left, right), axis=1)

        labelPath = image_path[:-4] + ".txt"

        currentPositionalData = PositionalData(getLabel(labelPath))
        # print("Relative Pose: ", currentPositionalData.timestamp)
        label = [None] * 4
        label[0], label[1], label[2] = currentPositionalData.getRelativeBlackPoseFromRedNormalizedRotation()

        if (labelPath[len(pathToFolder):(len(pathToFolder) + 10)] == "/null_data"):
            label[3] = 0
        else:
            label[3] = 1

        # print(label[3])
        groundTruth.append((label - mean) / std)

        frameNum += 1

    return stereo_dataset, groundTruth


def getPathsToLabels(dataset):
    labels = [None]*len(dataset)

    for i in range(len(labels)):
        labels[i] = dataset[i][:-3] + 'txt'

    return labels

def getLabel(pathToLabel):
    file = open(pathToLabel, "r")
    lines = file.readlines()

    return lines[-7:]

def Test():
    score = 0
    currentPositionalData = PositionalData(0, 3, 3, 0, 3, 1, 0)
    x, y, rz = currentPositionalData.getRelativeBlackPoseFromRed()
    if( x == 0 and y == -2 and rz == 0):
        print("Success")
        score = score + 1
    else:
        print("Failed")

    currentPositionalData = PositionalData(1, 3, 3, -0.5 * math.pi, 3, 1, 0)
    x, y, rz = currentPositionalData.getRelativeBlackPoseFromRed()
    if (x == 2 and y == 0 and rz == 1.5708):
        print("Success")
        score = score + 1
    else:
        print("Failed")

    currentPositionalData = PositionalData(1, 3, 3, -0.9*math.pi, 3, 1, 0)
    x, y, rz = currentPositionalData.getRelativeBlackPoseFromRed()
    if (x == 2 and y == 0 and rz == round(math.pi*0.9),4):
        print("Success")
        score = score + 1
    else:
        print("Failed")

    currentPositionalData = PositionalData(1, 3, 3, 0.4*math.pi, 3, 1, 0)
    x, y, rz = currentPositionalData.getRelativeBlackPoseFromRed()
    if (x == 2 and y == 0 and round(-math.pi*0.4),4):
        print("Success")
        score = score + 1
    else:
        print("Failed")

    currentPositionalData = PositionalData(2, 3, 3, -math.pi, 3, 1, math.pi / 2)
    x, y, rz = currentPositionalData.getRelativeBlackPoseFromRed()
    if (x == 0 and y == 2 and rz == -1.5708):
        print("Success")
        score = score + 1
    else:
        print("Failed")

    currentPositionalData = PositionalData(3, 3, 3, 0, 5, 5, 0)
    x, y, rz = currentPositionalData.getRelativeBlackPoseFromRed()
    if (x == 2 and y == 2 and rz == 0):
        print("Success")
        score = score + 1
    else:
        print("Failed")

    currentPositionalData = PositionalData(4, 3, 3, math.pi / 2, 5, 5, 0)
    x, y, rz = currentPositionalData.getRelativeBlackPoseFromRed()
    if (x == 2 and y == -2 and rz == -1.5708):
        print("Success")
        score = score + 1
    else:
        print("Failed")

    currentPositionalData = PositionalData(5, 3, 3, math.pi / 2, 5, 5, -1.5 * math.pi)
    x, y, rz = currentPositionalData.getRelativeBlackPoseFromRed()
    if (x == 2 and y == -2 and rz == 0):
        print("Success")
        score = score + 1
    else:
        print("Failed")

    if score == 8:
        print("All Tests Passed")

# def moveFilesToNewDirectory(pathToAllImages, pathToAllLables, newDirectory):
#
#     for image in pathToAllImages:
#         shutil.move(image, (newDirectory+image))
#
#     for label in pathToAllLables:
#         shutil.move(label, (newDirectory+label))
#
#     print("Completed")

if __name__ == "__main__":
    Test()
    # trainingImages = getPathsToImages(sys.argv[1], "good_data/**/*.jpg")
    #
    # pathsToLabels = getPathsToLabels(trainingImages)
    #
    # print("Total Good images in Train/Valid Dataset : ", str(len(trainingImages)))
    #
    # rel_position = np.zeros(shape=(len(trainingImages), 2))
    # orientations = np.zeros(shape=(len(trainingImages), 1))
    #
    # badCounter = 0
    # for i in range(len(trainingImages)):
    #     label = getLabel(pathsToLabels[i])
    #     try:
    #         currentPositionalData = PositionalData(label[0], label[1], label[2], label[3], label[4], label[5], label[6])
    #         # print("Relative Pose: ", currentPositionalData.timestamp)
    #         x_rel_pos, y_rel_pos, rel_rot = currentPositionalData.getRelativeBlackPoseFromRed()
    #
    #         if x_rel_pos < 0.1:
    #             print(pathsToLabels[i])
    #             exit(0)
    #
    #         if y_rel_pos < -5 or y_rel_pos > 5:
    #             print(pathsToLabels[i])
    #             exit(0)
    #
    #         rel_position[i][0] = x_rel_pos
    #         rel_position[i][1] = y_rel_pos
    #         orientations[i][0] = rel_rot
    #     except:
    #         badCounter = badCounter + 1
    #         print(trainingImages[i])
    #         #print((pathsToLabels[i]))
    #
    #
    # print("Bad files ")
    # print(badCounter)
    # plt.hist(x=orientations, density=False, bins=100)
    # plt.ylabel("Number of occurances")
    # plt.xlabel("Relative Orientation")
    # plt.show()
    #
    # plt.hist(x=rel_position[:, 0], density=False, bins=100)
    # plt.ylabel("Number of occurances")
    # plt.xlabel("X Distance")
    # plt.show()
    #
    # plt.hist(x=rel_position[:, 1], density=False, bins=100)
    # plt.ylabel("Number of occurances")
    # plt.xlabel("Y Distance")
    # plt.show()
    #
    # plt.scatter(x=rel_position[:, 1], y=rel_position[:, 0])
    # plt.gca().invert_xaxis()
    # plt.ylabel("Distance in X Axis")
    # plt.xlabel("Distance in Y Axis")
    # plt.show()



