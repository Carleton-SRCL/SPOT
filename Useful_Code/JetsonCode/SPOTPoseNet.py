from tensorflow.keras.layers import Conv2D, MaxPooling2D, Input, Dense, Flatten, Dropout, BatchNormalization
from tensorflow.keras.models import Model
import tensorflow.keras.optimizers.schedules as Schedules
from tensorflow.keras import losses
import tensorflow as tf
from tensorflow.keras import optimizers
from tensorboard.plugins.hparams import api as hp
import CustomVideoDataGenerator
import CustomImageDataGenerator
from tensorflow.keras.callbacks import EarlyStopping, ModelCheckpoint
from tensorflow.keras.preprocessing.image import ImageDataGenerator
import numpy as np
from random import shuffle
from time import localtime, strftime
from tensorflow.keras import backend
import sys

# Image Augmentation Generator
imgGen = ImageDataGenerator(brightness_range=[0.75, 1.25], samplewise_center=False, samplewise_std_normalization=False)

# Hyperparameters
HP_IMAGE_SIZE = hp.HParam('num_units', hp.Discrete([448]))
HP_DROPOUT = hp.HParam('dropout', hp.Discrete([0.4]))
HP_OPTIMIZER = hp.HParam('optimizer', hp.Discrete(['adam']))
HP_FILTER_SIZE = hp.HParam('filter_size', hp.Discrete([7]))

leaky_relu = tf.keras.layers.LeakyReLU(alpha=0.1)


def run_video(run_dir, hparams, path, epochs_to_wait_for_improve, batch_size, num_epochs):
    with tf.summary.create_file_writer(run_dir).as_default():
        hp.hparams(hparams)  # record the values used in this trial
        loss = pose_model_video(path, epochs_to_wait_for_improve, batch_size, num_epochs, hparams, run_dir)
        tf.summary.scalar('loss', loss, step=1)


def run_images(run_dir, hparams, path, epochs_to_wait_for_improve, batch_size, num_epochs, training_set, validation_set):
    with tf.summary.create_file_writer(run_dir).as_default():
        hp.hparams(hparams)  # record the values used in this trial
        loss = pose_model_images(path, epochs_to_wait_for_improve, batch_size, num_epochs, hparams, run_dir, training_set, validation_set)
        tf.summary.scalar('loss', loss, step=1)


def video_trainer():
    backend.clear_session()
    path = sys.argv[1]
    epochs_to_wait_for_improve = int(sys.argv[2])
    batch_size = int(sys.argv[3])
    num_epochs = int(sys.argv[4])

    with tf.summary.create_file_writer('logs/hparam_tuning').as_default():
        hp.hparams_config(
            hparams=[HP_IMAGE_SIZE, HP_DROPOUT, HP_OPTIMIZER],
            metrics=[
                hp.Metric('x_out', group="validation", display_name='x_accuracy_validation'),
                hp.Metric('y_out', group="validation", display_name='y_accuracy_validation'),
                hp.Metric('yaw_out', group="validation", display_name='yaw_accuracy_validation'),
                hp.Metric('class_out', group="validation", display_name='class_accuracy_validation')
            ],
        )

    session_num = 0

    for img_size in HP_IMAGE_SIZE.domain.values:
        for dropout_rate in HP_DROPOUT.domain.values:
            for optimizer in HP_OPTIMIZER.domain.values:
                hparams = {
                    HP_IMAGE_SIZE: img_size,
                    HP_DROPOUT: dropout_rate,
                    HP_OPTIMIZER: optimizer,
                }
                run_name = "run-%d" % session_num
                print('--- Starting trial: %s' % run_name)
                print({h.name: hparams[h] for h in hparams})
                run_video('logs/hparam_tuning/' + run_name, hparams, path, epochs_to_wait_for_improve, batch_size, num_epochs)
                session_num += 1


def image_trainer():
    backend.clear_session()
    path = sys.argv[1]
    epochs_to_wait_for_improve = int(sys.argv[2])
    batch_size = int(sys.argv[3])
    num_epochs = int(sys.argv[4])

    goodDataset, nullDataset = CustomImageDataGenerator.getDataset(path)

    training_set, validation_set = CustomImageDataGenerator.datasetShuffleAndSplit(goodDataset, nullDataset, 0.70)

    with tf.summary.create_file_writer('logs/hparam_tuning').as_default():
        hp.hparams_config(
            hparams=[HP_FILTER_SIZE, HP_IMAGE_SIZE, HP_DROPOUT, HP_OPTIMIZER],
            metrics=[
                hp.Metric('x_out', group="validation", display_name='x_accuracy_validation'),
                hp.Metric('y_out', group="validation", display_name='y_accuracy_validation'),
                hp.Metric('yaw_out', group="validation", display_name='yaw_accuracy_validation'),
                hp.Metric('class_out', group="validation", display_name='class_accuracy_validation')
            ],
        )

    session_num = 0

    for img_size in HP_IMAGE_SIZE.domain.values:
        for dropout_rate in HP_DROPOUT.domain.values:
            for optimizer in HP_OPTIMIZER.domain.values:
                for filter_size in HP_FILTER_SIZE.domain.values:
                    hparams = {
                        HP_FILTER_SIZE: filter_size,
                        HP_IMAGE_SIZE: img_size,
                        HP_DROPOUT: dropout_rate,
                        HP_OPTIMIZER: optimizer,
                    }
                    run_name = "run-%d" % session_num
                    print('--- Starting trial: %s' % run_name)
                    print({h.name: hparams[h] for h in hparams})
                    run_images('logs/hparam_tuning/' + run_name, hparams, path, epochs_to_wait_for_improve, batch_size, num_epochs, training_set, validation_set)
                    session_num += 1


def generator_video_stereo_input_batches(video_tuple, batch_size, x_size, y_size, mean, std):
    while True:
        start = 0
        shuffle(video_tuple)
        end = (len(video_tuple) - batch_size)
        while start < end:
            video_batch = video_tuple[start:(start + batch_size)]
            start += batch_size

            stereo, y = CustomVideoDataGenerator.generateGrayTimeSeriesDataFromVideoBatch(video_batch, x_size, y_size, mean, std)

            gen1 = imgGen.flow(stereo, y, batch_size=batch_size, shuffle=True)
            out = gen1.next()

            stacked_stereo = np.concatenate(np.split(out[0], 2, axis=2), axis=3)
            yield [stacked_stereo], [out[1][:, [0, 3]],
                                     out[1][:, [1, 3]],
                                     out[1][:, [2, 3]],
                                     out[1][:,
                                     3]]  # a tuple with two numpy arrays with batch_size samples


def generator_stereo_images(pathToFolder, image_tuple, batch_size, x_size, y_size, mean, std):
    while True:
        start = 0
        end = (len(image_tuple))
        while start < end:
            image_batch = image_tuple[start:(start + batch_size)]
            start += batch_size

            stereo, y = CustomImageDataGenerator.generateImageBatchDataAndLabels(pathToFolder, image_batch, x_size, y_size, mean, std)

            gen1 = imgGen.flow(stereo, y, batch_size=batch_size, shuffle=True)
            out = gen1.next()

            stacked_stereo = np.concatenate(np.split(out[0], 2, axis=2), axis=3)
            yield [stacked_stereo], [out[1][:, [0, 3]],
                                     out[1][:, [1, 3]],
                                     out[1][:, [2, 3]],
                                     out[1][:, 3]]  # a tuple with two numpy arrays with batch_size samples

def generator_stereo_images(pathToFolder, image_tuple, batch_size, x_size, y_size, mean, std):
    while True:
        start = 0
        end = (len(image_tuple))
        while start < end:
            image_batch = image_tuple[start:(start + batch_size)]
            start += batch_size

            stereo, y = CustomImageDataGenerator.generateImageBatchDataAndLabels(pathToFolder, image_batch, x_size, y_size, mean, std)

            gen1 = imgGen.flow(stereo, y, batch_size=batch_size, shuffle=True)
            out = gen1.next()

            stacked_stereo = np.concatenate(np.split(out[0], 2, axis=2), axis=3)
            yield [stacked_stereo], [out[1][:, [0, 3]],
                                     out[1][:, [1, 3]],
                                     out[1][:, [2, 3]],
                                     out[1][:, 3]]  # a tuple with two numpy arrays with batch_size samples


def generateVideoData(path, batch_size, image_x_size, image_y_size):
    dataset = CustomVideoDataGenerator.getDataset(path)

    training_set, validation_set = CustomVideoDataGenerator.datasetShuffleAndSplit(dataset, 0.70)

    training_labels = np.zeros(shape=(len(training_set), 4))
    valid_labels = np.zeros(shape=(len(validation_set), 4))

    count = 0
    print("Training data size")
    print(len(training_set))

    print("Validation data size")
    print(len(validation_set))

    for trainingData in training_set:
        path = trainingData[0][:-4] + ".h5"
        label = CustomVideoDataGenerator.getGroundTruth(path, trainingData[1])
        training_labels[count] = label
        count += 1

    count = 0
    for validData in validation_set:
        path = validData[0][:-4] + ".h5"
        label = CustomVideoDataGenerator.getGroundTruth(path, validData[1])
        valid_labels[count] = label
        count += 1

    print("Training Labels Size")
    print(len(training_labels))
    mean = np.zeros(4)
    std = np.ones(4)
    normData, mean[:-1], std[:-1] = CustomVideoDataGenerator.normalizeData(training_labels[:, 0:3])

    std = np.ones(4)  # just want to zero shift the data
    mean_detc = (training_labels[:, 3]).mean(axis=0)
    mean_valid = (valid_labels[:, 3]).mean(axis=0)

    print("Mean")
    print(mean)
    print("Std")
    print(std)

    print("Mean Training Detection")
    print(mean_detc)

    print("Mean Validation Detection")
    print(mean_valid)

    steps_per_epoch_train = len(training_set) / batch_size
    steps_per_epoch_valid = len(validation_set) / batch_size

    gen_stereo_train = generator_video_stereo_input_batches(training_set, batch_size, image_x_size, image_y_size, mean, std)
    gen_stereo_valid = generator_video_stereo_input_batches(validation_set, batch_size, image_x_size, image_y_size, mean, std)

    return gen_stereo_train, gen_stereo_valid, steps_per_epoch_train, steps_per_epoch_valid,


def generateImageData(pathToFolder, batch_size, image_x_size, image_y_size, training_set, validation_set):

    training_labels = np.zeros(shape=(len(training_set), 4))
    valid_labels = np.zeros(shape=(len(validation_set), 4))

    count = 0
    print("Training data size")
    print(len(training_set))

    print("Validation data size")
    print(len(validation_set))

    for trainingData in training_set:
        path = trainingData[:-4] + ".txt"
        decodedData = CustomImageDataGenerator.PositionalData(CustomImageDataGenerator.getLabel(path))
        label = [None]*4
        label[0], label[1], label[2] = decodedData.getRelativeBlackPoseFromRedNormalizedRotation()
        label[3] = 1

        # print(path[len(pathToFolder):(len(pathToFolder) + 10)])
        if (path[len(pathToFolder):(len(pathToFolder) + 10)] == "/null_data"):
            label[3] = 0

        training_labels[count] = label
        count += 1

    count = 0
    for validData in validation_set:
        path = validData[:-4] + ".txt"
        decodedData = CustomImageDataGenerator.PositionalData(CustomImageDataGenerator.getLabel(path))
        label = [None]*4
        label[0], label[1], label[2] = decodedData.getRelativeBlackPoseFromRedNormalizedRotation()
        label[3] = 1
        if (path[len(pathToFolder):(len(pathToFolder) + 10)] == "/null_data"):
            label[3] = 0

        valid_labels[count] = label
        count += 1

    print("Training Labels Size")
    print(len(training_labels))
    mean = np.zeros(4)
    std = np.ones(4)
    normData, mean[:-2], std[:-2] = CustomImageDataGenerator.normalizeData(training_labels[:, 0:2])

    std = np.ones(4)  # just want to zero shift the data
    mean_detc = (training_labels[:, 3]).mean(axis=0)
    mean_valid = (valid_labels[:, 3]).mean(axis=0)

    print("Mean")
    print(mean)
    print("Std")
    print(std)

    print("Mean Training Detection (GOOD DATA, NOT NULL)")
    print(mean_detc)

    print("Mean Validation Detection (GOOD DATA, NOT NULL)")
    print(mean_valid)

    steps_per_epoch_train = len(training_set) / batch_size
    steps_per_epoch_valid = len(validation_set) / batch_size

    gen_stereo_train = generator_stereo_images(pathToFolder, training_set, batch_size, image_x_size, image_y_size, mean, std)
    gen_stereo_valid = generator_stereo_images(pathToFolder, validation_set, batch_size, image_x_size, image_y_size, mean, std)

    return gen_stereo_train, gen_stereo_valid, steps_per_epoch_train, steps_per_epoch_valid,


# custom loss functions, if there is no object in front then loss = 0 which prevents update to model for incorrect example

def custom_loss(y_true, y_pred):
    return losses.mean_squared_error(y_true[:, 0] * y_true[:, 1], y_pred[:, 0] * y_true[:, 1])


def custom_metric(y_true, y_pred):
    return losses.mean_absolute_error(y_true[:, 0] * y_true[:, 1], y_pred[:, 0] * y_true[:, 1])


def standard_layer(image, num_filters, filter_size, filter_stride, pooling_size, pooling_stride):
    cnn = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(
        image)
    pool = MaxPooling2D(pooling_size, strides=pooling_stride)(cnn)
    return pool


def nn_dense_layer(layer_input, layer_output, num_activations, output_activation, dropout_rate, name):
    l1 = Dense(num_activations, activation=leaky_relu)(layer_input)
    l1d = Dropout(dropout_rate)(l1)

    return Dense(layer_output, activation=output_activation, name=name)(l1d)


# stereo based convolutions
# 2 channel input left/right image instead of stereo layers

def getModelArchitecture(image_x_size, image_y_size, hparams):
    ######
    num_filters = 32
    filter_stride = (1, 1)
    filter_size = (3, 3)
    pooling_stride = (2, 2)
    pooling_size = (2, 2)

    image_input = Input(shape=(image_y_size, image_x_size, 2))

    input_layer = standard_layer(image_input, num_filters, filter_size, filter_stride, pooling_size, pooling_stride)

    num_filters = 64
    filter_stride = (1, 1)
    filter_size = (3, 3)
    pooling_stride = (2, 2)
    pooling_size = (2, 2)

    l1 = standard_layer(input_layer, num_filters, filter_size, filter_stride, pooling_size, pooling_stride)

    ######
    num_filters = 128
    filter_stride = (1, 1)
    filter_size = (3, 3)

    l2 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l1)

    num_filters = 64
    filter_stride = (1, 1)
    filter_size = (1, 1)

    l3 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l2)

    num_filters = 128
    filter_stride = (1, 1)
    filter_size = (3, 3)
    pooling_stride = (2, 2)
    pooling_size = (2, 2)

    l4 = standard_layer(l3, num_filters, filter_size, filter_stride, pooling_size, pooling_stride)

    #####
    num_filters = 256
    filter_stride = (1, 1)
    filter_size = (3, 3)

    l5 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l4)

    num_filters = 128
    filter_stride = (1, 1)
    filter_size = (1, 1)

    l6 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l5)

    num_filters = 256
    filter_stride = (1, 1)
    filter_size = (3, 3)
    pooling_stride = (2, 2)
    pooling_size = (2, 2)

    l7 = standard_layer(l6, num_filters, filter_size, filter_stride, pooling_size, pooling_stride)

    #####
    num_filters = 512
    filter_stride = (1, 1)
    filter_size = (3, 3)

    l8 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l7)

    num_filters = 256
    filter_stride = (1, 1)
    filter_size = (1, 1)

    l9 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l8)

    num_filters = 512
    filter_stride = (1, 1)
    filter_size = (3, 3)
    pooling_stride = (2, 2)
    pooling_size = (2, 2)

    l10 = standard_layer(l9, num_filters, filter_size, filter_stride, pooling_size, pooling_stride)

    num_filters = 1024
    filter_stride = (1, 1)
    filter_size = (3, 3)

    l11 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l10)

    flatten = Flatten()(l11)

    x_pos_out = nn_dense_layer(flatten, 1, 64, 'linear', hparams[HP_DROPOUT], 'x_out')
    y_pos_out = nn_dense_layer(flatten, 1, 64, 'linear', hparams[HP_DROPOUT], 'y_out')
    yaw_pos_out = nn_dense_layer(flatten, 1, 64, 'linear', hparams[HP_DROPOUT], 'yaw_out')
    class_out = nn_dense_layer(flatten, 1, 8, 'sigmoid', hparams[HP_DROPOUT], 'class_out')

    out_model = Model(inputs=[image_input], outputs=[x_pos_out, y_pos_out, yaw_pos_out, class_out])
    out_model.summary()

    return out_model

def getModelArchitecture2(image_x_size, image_y_size, hparams):
    ######
    num_filters = 32
    filter_stride = (1, 1)
    filter_size = (3, 3)
    pooling_stride = (2, 2)
    pooling_size = (2, 2)

    image_input = Input(shape=(image_y_size, image_x_size, 2))

    input_layer = standard_layer(image_input, num_filters, filter_size, filter_stride, pooling_size, pooling_stride)

    num_filters = 64
    filter_stride = (1, 1)
    filter_size = (3, 3)
    pooling_stride = (2, 2)
    pooling_size = (2, 2)

    l1 = standard_layer(input_layer, num_filters, filter_size, filter_stride, pooling_size, pooling_stride)

    ######
    num_filters = 128
    filter_stride = (1, 1)
    filter_size = (3, 3)

    l2 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l1)

    num_filters = 64
    filter_stride = (1, 1)
    filter_size = (1, 1)

    l3 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l2)

    num_filters = 128
    filter_stride = (1, 1)
    filter_size = (3, 3)
    pooling_stride = (2, 2)
    pooling_size = (2, 2)

    l4 = standard_layer(l3, num_filters, filter_size, filter_stride, pooling_size, pooling_stride)

    #####
    num_filters = 256
    filter_stride = (1, 1)
    filter_size = (3, 3)

    l5 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l4)

    num_filters = 128
    filter_stride = (1, 1)
    filter_size = (1, 1)

    l6 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l5)

    num_filters = 256
    filter_stride = (1, 1)
    filter_size = (3, 3)
    pooling_stride = (2, 2)
    pooling_size = (2, 2)

    l7 = standard_layer(l6, num_filters, filter_size, filter_stride, pooling_size, pooling_stride)

    #####
    num_filters = 512
    filter_stride = (1, 1)
    filter_size = (3, 3)

    l8 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l7)

    num_filters = 256
    filter_stride = (1, 1)
    filter_size = (1, 1)

    l9 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l8)

    num_filters = 512
    filter_stride = (1, 1)
    filter_size = (3, 3)
    pooling_stride = (2, 2)
    pooling_size = (2, 2)

    l10 = standard_layer(l9, num_filters, filter_size, filter_stride, pooling_size, pooling_stride)

    num_filters = 1024
    filter_stride = (1, 1)
    filter_size = (3, 3)

    l11 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l10)

    flatten = Flatten()(l11)

    x_pos_out = nn_dense_layer(flatten, 1, 64, 'linear', hparams[HP_DROPOUT], 'x_out')
    y_pos_out = nn_dense_layer(flatten, 1, 64, 'linear', hparams[HP_DROPOUT], 'y_out')
    yaw1 = nn_dense_layer(flatten, 64, 256, 'linear', hparams[HP_DROPOUT], 'yaw1')
    yaw_pos_out = nn_dense_layer(yaw1, 1, 8, 'linear', hparams[HP_DROPOUT], 'yaw_out')
    class_out = nn_dense_layer(flatten, 1, 8, 'sigmoid', hparams[HP_DROPOUT], 'class_out')

    out_model = Model(inputs=[image_input], outputs=[x_pos_out, y_pos_out, yaw_pos_out, class_out])
    out_model.summary()

    return out_model

def getModelArchitecture2Test(image_x_size, image_y_size):
    ######
    num_filters = 32
    filter_stride = (1, 1)
    filter_size = (3, 3)
    pooling_stride = (2, 2)
    pooling_size = (2, 2)

    image_input = Input(shape=(image_y_size, image_x_size, 2))

    input_layer = standard_layer(image_input, num_filters, filter_size, filter_stride, pooling_size, pooling_stride)

    num_filters = 64
    filter_stride = (1, 1)
    filter_size = (3, 3)
    pooling_stride = (2, 2)
    pooling_size = (2, 2)

    l1 = standard_layer(input_layer, num_filters, filter_size, filter_stride, pooling_size, pooling_stride)

    ######
    num_filters = 128
    filter_stride = (1, 1)
    filter_size = (3, 3)

    l2 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l1)

    num_filters = 64
    filter_stride = (1, 1)
    filter_size = (1, 1)

    l3 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l2)

    num_filters = 128
    filter_stride = (1, 1)
    filter_size = (3, 3)
    pooling_stride = (2, 2)
    pooling_size = (2, 2)

    l4 = standard_layer(l3, num_filters, filter_size, filter_stride, pooling_size, pooling_stride)

    #####
    num_filters = 256
    filter_stride = (1, 1)
    filter_size = (3, 3)

    l5 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l4)

    num_filters = 128
    filter_stride = (1, 1)
    filter_size = (1, 1)

    l6 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l5)

    num_filters = 256
    filter_stride = (1, 1)
    filter_size = (3, 3)
    pooling_stride = (2, 2)
    pooling_size = (2, 2)

    l7 = standard_layer(l6, num_filters, filter_size, filter_stride, pooling_size, pooling_stride)

    #####
    num_filters = 512
    filter_stride = (1, 1)
    filter_size = (3, 3)

    l8 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l7)

    num_filters = 256
    filter_stride = (1, 1)
    filter_size = (1, 1)

    l9 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l8)

    num_filters = 512
    filter_stride = (1, 1)
    filter_size = (3, 3)
    pooling_stride = (2, 2)
    pooling_size = (2, 2)

    l10 = standard_layer(l9, num_filters, filter_size, filter_stride, pooling_size, pooling_stride)

    num_filters = 1024
    filter_stride = (1, 1)
    filter_size = (3, 3)

    l11 = Conv2D(num_filters, kernel_size=filter_size, strides=filter_stride, padding="same", activation=leaky_relu)(l10)

    flatten = Flatten()(l11)

    x_pos_out = nn_dense_layer(flatten, 1, 64, 'linear', 0.4, 'x_out')
    y_pos_out = nn_dense_layer(flatten, 1, 64, 'linear', 0.4, 'y_out')
    yaw1 = nn_dense_layer(flatten, 64, 256, 'linear', 0.4, 'yaw1')
    yaw_pos_out = nn_dense_layer(yaw1, 1, 8, 'linear', 0.4, 'yaw_out')
    class_out = nn_dense_layer(flatten, 1, 8, 'sigmoid', 0.4, 'class_out')

    out_model = Model(inputs=[image_input], outputs=[x_pos_out, y_pos_out, yaw_pos_out, class_out])
    out_model.summary()

    return out_model

def getModelArchitecture3(image_x_size, image_y_size, hparams):
    ######
    image_input = Input(shape=(image_y_size, image_x_size, 2))

    l0_33 = Conv2D(32, kernel_size=(3, 3), strides=(1, 1), padding="same")(image_input)
    l0l_33 = tf.keras.layers.LeakyReLU(alpha=0.1)(l0_33)

    l0_77 = Conv2D(32, kernel_size=(hparams[HP_FILTER_SIZE], hparams[HP_FILTER_SIZE]), strides=(1, 1), padding="same")(image_input)
    l0l_77 = tf.keras.layers.LeakyReLU(alpha=0.1)(l0_77)

    l0_concate = tf.keras.layers.concatenate([l0l_33, l0l_77])

    l1 = Conv2D(128, kernel_size=(3, 3), strides=(1, 1), padding="same")(l0_concate)
    l1l = tf.keras.layers.LeakyReLU(alpha=0.1)(l1)
    l1p = MaxPooling2D((2, 2), strides=(2, 2))(l1l)

    l2 = Conv2D(256, kernel_size=(3, 3), strides=(1, 1), padding="same")(l1p)
    l2l = tf.keras.layers.LeakyReLU(alpha=0.1)(l2)

    l3 = Conv2D(128, kernel_size=(1, 1), strides=(1, 1), padding="same")(l2l)
    l3l = tf.keras.layers.LeakyReLU(alpha=0.1)(l3)

    l4 = Conv2D(256, kernel_size=(3, 3), strides=(1, 1), padding="same")(l3l)
    l4l = tf.keras.layers.LeakyReLU(alpha=0.1)(l4)
    l4p = MaxPooling2D((2, 2), strides=(2, 2))(l4l)

    l4 = Conv2D(512, kernel_size=(3, 3), strides=(1, 1), padding="same")(l4p)
    l4l = tf.keras.layers.LeakyReLU(alpha=0.1)(l4)

    l5 = Conv2D(256, kernel_size=(1, 1), strides=(1, 1), padding="same")(l4l)
    l5l = tf.keras.layers.LeakyReLU(alpha=0.1)(l5)

    l6 = Conv2D(512, kernel_size=(3, 3), strides=(1, 1), padding="same")(l5l)
    l6l = tf.keras.layers.LeakyReLU(alpha=0.1)(l6)
    l6p = MaxPooling2D((2, 2), strides=(2, 2))(l6l)

    l7 = Conv2D(1024, kernel_size=(3, 3), strides=(1, 1), padding="same")(l6p)
    l7l = tf.keras.layers.LeakyReLU(alpha=0.1)(l7)
    l7p = MaxPooling2D((2, 2), strides=(2, 2))(l7l)

    l8 = Conv2D(1024, kernel_size=(3, 3), strides=(1, 1), padding="same")(l7p)
    l8l = tf.keras.layers.LeakyReLU(alpha=0.1)(l8)
    l8p = MaxPooling2D((2, 2), strides=(2, 2))(l8l)

    l9 = Conv2D(512, kernel_size=(3, 3), strides=(1, 1), padding="same")(l8p)
    l9l = tf.keras.layers.LeakyReLU(alpha=0.1)(l9)
    l9p = tf.keras.layers.AveragePooling2D((2, 2), strides=(2, 2))(l9l)

    flatten = Flatten()(l9p)

    x_pos_0 = Dense(64)(flatten)
    x_pos_1 = Dropout(hparams[HP_DROPOUT])(x_pos_0)
    x_pos_out = Dense(1, activation='linear', name='x_out')(x_pos_1)

    y_pos_0 = Dense(64)(flatten)
    y_pos_1 = Dropout(hparams[HP_DROPOUT])(y_pos_0)
    y_pos_out = Dense(1, activation='linear', name='y_out')(y_pos_1)

    yaw_pos_0 = Dense(256)(flatten)
    yaw_pos_1 = Dropout(hparams[HP_DROPOUT])(yaw_pos_0)
    yaw_pos_out = Dense(1, activation='sigmoid', name='yaw_out')(yaw_pos_1)

    class_out_0 = Dense(32)(flatten)
    class_out_1 = Dropout(hparams[HP_DROPOUT])(class_out_0)
    class_out = Dense(1, activation='sigmoid', name='class_out')(class_out_1)

    out_model = Model(inputs=[image_input], outputs=[x_pos_out, y_pos_out, yaw_pos_out, class_out])
    out_model.summary()

    return out_model

def getModelArchitecture3Test(image_x_size, image_y_size, filter_size):
    ######
    image_input = Input(shape=(image_y_size, image_x_size, 2))

    l0_33 = Conv2D(32, kernel_size=(3, 3), strides=(1, 1), padding="same")(image_input)
    l0l_33 = tf.keras.layers.LeakyReLU(alpha=0.1)(l0_33)

    l0_77 = Conv2D(32, kernel_size=(filter_size, filter_size), strides=(1, 1), padding="same")(image_input)
    l0l_77 = tf.keras.layers.LeakyReLU(alpha=0.1)(l0_77)

    l0_concate = tf.keras.layers.concatenate([l0l_33, l0l_77])

    l1 = Conv2D(128, kernel_size=(3, 3), strides=(1, 1), padding="same")(l0_concate)
    l1l = tf.keras.layers.LeakyReLU(alpha=0.1)(l1)
    l1p = MaxPooling2D((2, 2), strides=(2, 2))(l1l)

    l2 = Conv2D(256, kernel_size=(3, 3), strides=(1, 1), padding="same")(l1p)
    l2l = tf.keras.layers.LeakyReLU(alpha=0.1)(l2)

    l3 = Conv2D(128, kernel_size=(1, 1), strides=(1, 1), padding="same")(l2l)
    l3l = tf.keras.layers.LeakyReLU(alpha=0.1)(l3)

    l4 = Conv2D(256, kernel_size=(3, 3), strides=(1, 1), padding="same")(l3l)
    l4l = tf.keras.layers.LeakyReLU(alpha=0.1)(l4)
    l4p = MaxPooling2D((2, 2), strides=(2, 2))(l4l)

    l4 = Conv2D(512, kernel_size=(3, 3), strides=(1, 1), padding="same")(l4p)
    l4l = tf.keras.layers.LeakyReLU(alpha=0.1)(l4)

    l5 = Conv2D(256, kernel_size=(1, 1), strides=(1, 1), padding="same")(l4l)
    l5l = tf.keras.layers.LeakyReLU(alpha=0.1)(l5)

    l6 = Conv2D(512, kernel_size=(3, 3), strides=(1, 1), padding="same")(l5l)
    l6l = tf.keras.layers.LeakyReLU(alpha=0.1)(l6)
    l6p = MaxPooling2D((2, 2), strides=(2, 2))(l6l)

    l7 = Conv2D(1024, kernel_size=(3, 3), strides=(1, 1), padding="same")(l6p)
    l7l = tf.keras.layers.LeakyReLU(alpha=0.1)(l7)
    l7p = MaxPooling2D((2, 2), strides=(2, 2))(l7l)

    l8 = Conv2D(1024, kernel_size=(3, 3), strides=(1, 1), padding="same")(l7p)
    l8l = tf.keras.layers.LeakyReLU(alpha=0.1)(l8)
    l8p = MaxPooling2D((2, 2), strides=(2, 2))(l8l)

    l9 = Conv2D(512, kernel_size=(3, 3), strides=(1, 1), padding="same")(l8p)
    l9l = tf.keras.layers.LeakyReLU(alpha=0.1)(l9)
    l9p = tf.keras.layers.AveragePooling2D((2, 2), strides=(2, 2))(l9l)

    flatten = Flatten()(l9p)

    x_pos_0 = Dense(64)(flatten)
    x_pos_1 = Dropout(0.4)(x_pos_0)
    x_pos_out = Dense(1, activation='linear', name='x_out')(x_pos_1)

    y_pos_0 = Dense(64)(flatten)
    y_pos_1 = Dropout(0.4)(y_pos_0)
    y_pos_out = Dense(1, activation='linear', name='y_out')(y_pos_1)

    yaw_pos_0 = Dense(256)(flatten)
    yaw_pos_1 = Dropout(0.4)(yaw_pos_0)
    yaw_pos_out = Dense(1, activation='sigmoid', name='yaw_out')(yaw_pos_1)

    class_out_0 = Dense(32)(flatten)
    class_out_1 = Dropout(0.4)(class_out_0)
    class_out = Dense(1, activation='sigmoid', name='class_out')(class_out_1)

    out_model = Model(inputs=[image_input], outputs=[x_pos_out, y_pos_out, yaw_pos_out, class_out])
    out_model.summary()

    return out_model

def pose_model_video(path, epochs_to_wait_for_improve, batch_size, num_epochs, hparams, logdir):
    currentTime = strftime("%Y-%m-%d-%H:%M", localtime())

    image_x_size = hparams[HP_IMAGE_SIZE]
    image_y_size = hparams[HP_IMAGE_SIZE]

    gen_stereo_train, gen_stereo_valid, steps_per_epoch_train, steps_per_epoch_valid = generateVideoData(path,
                                                                                                    batch_size,
                                                                                                    image_x_size,
                                                                                                    image_y_size)
    model = getModelArchitecture(image_x_size, image_y_size, hparams)

    lr_schedule = Schedules.ExponentialDecay(initial_learning_rate=1e-3,
                                             decay_steps=steps_per_epoch_train,
                                             decay_rate=0.9)

    train_optimizer = optimizers.SGD(learning_rate=lr_schedule, momentum=0.9)

    if hparams[HP_OPTIMIZER] == 'adam':
        lr_schedule = Schedules.ExponentialDecay(initial_learning_rate=1e-6,
                                                 decay_steps=steps_per_epoch_train,
                                                 decay_rate=0.9)
        train_optimizer = optimizers.Adam(learning_rate=lr_schedule)

    model.compile(optimizer=train_optimizer,
                  loss={'x_out': custom_loss, 'y_out': custom_loss, 'yaw_out': custom_loss,
                        'class_out': 'binary_crossentropy'},
                  loss_weights={'x_out': 1., 'y_out': 1., 'yaw_out': 1., 'class_out': 0.75},
                  metrics={'x_out': custom_metric, 'y_out': custom_metric, 'yaw_out': custom_metric,
                           'class_out': 'accuracy'})

    early_stopping_callback = EarlyStopping(monitor='val_loss',
                                            patience=epochs_to_wait_for_improve)

    checkpoint_callback = ModelCheckpoint('Experiment_' + currentTime + '.h5',
                                          monitor='val_loss',
                                          save_weights_only=False,
                                          verbose=1,
                                          save_best_only=True,
                                          mode='min')

    history = model.fit(gen_stereo_train,
                        steps_per_epoch=steps_per_epoch_train,
                        epochs=num_epochs,
                        validation_data=gen_stereo_valid,
                        validation_steps=steps_per_epoch_valid,
                        use_multiprocessing=False,
                        callbacks=[early_stopping_callback,
                                   checkpoint_callback,
                                   tf.keras.callbacks.TensorBoard(logdir),  # log metrics
                                   hp.KerasCallback(logdir, hparams),  # log hparams
                                   ])

    val_losses = history.history['val_loss']

    return min(val_losses)

def pose_model_images(path, epochs_to_wait_for_improve, batch_size, num_epochs, hparams, logdir, training_set, validation_set):
    currentTime = strftime("%Y-%m-%d-%H:%M", localtime())

    image_x_size = hparams[HP_IMAGE_SIZE]
    image_y_size = hparams[HP_IMAGE_SIZE]

    gen_stereo_train, gen_stereo_valid, steps_per_epoch_train, steps_per_epoch_valid = generateImageData(path,
                                                                                                         batch_size,
                                                                                                         image_x_size,
                                                                                                         image_y_size,
                                                                                                         training_set,
                                                                                                         validation_set)
    model = getModelArchitecture3(image_x_size, image_y_size, hparams)

    lr_schedule = Schedules.ExponentialDecay(initial_learning_rate=1e-5,
                                             decay_steps=steps_per_epoch_train,
                                             decay_rate=0.9)

    train_optimizer = optimizers.SGD(learning_rate=lr_schedule)

    if hparams[HP_OPTIMIZER] == 'adam':
        print('Using Adam Optimizer')
        train_optimizer = optimizers.Adam(learning_rate=lr_schedule)

    model.compile(optimizer=train_optimizer,
                  loss={'x_out': custom_loss, 'y_out': custom_loss, 'yaw_out': custom_loss,
                        'class_out': 'binary_crossentropy'},
                  loss_weights={'x_out': 1., 'y_out': 1., 'yaw_out': 1., 'class_out': 0.75},
                  metrics={'x_out': custom_metric, 'y_out': custom_metric, 'yaw_out': custom_metric,
                           'class_out': 'accuracy'})

    early_stopping_callback = EarlyStopping(monitor='val_loss',
                                            patience=epochs_to_wait_for_improve)

    checkpoint_callback = ModelCheckpoint('Experiment_' + currentTime + '.h5',
                                          monitor='val_loss',
                                          save_weights_only=False,
                                          verbose=1,
                                          save_best_only=True,
                                          mode='min')

    history = model.fit(gen_stereo_train,
                        steps_per_epoch=steps_per_epoch_train,
                        epochs=num_epochs,
                        validation_data=gen_stereo_valid,
                        validation_steps=steps_per_epoch_valid,
                        use_multiprocessing=False,
                        callbacks=[early_stopping_callback,
                                   checkpoint_callback,
                                   tf.keras.callbacks.TensorBoard(logdir),  # log metrics
                                   hp.KerasCallback(logdir, hparams),  # log hparams
                                   ])

    val_losses = history.history['val_loss']

    return min(val_losses)


def Main():
    #video_trainer()
    image_trainer()
    #generateImageData('/Users/FrankDespond/Desktop/Test_code', 30, 400, 400)
    # getModelArchitecture3Test(320, 320, 3)
    # getModelArchitecture3Test(320, 320, 7)

if __name__ == "__main__":
    Main()
