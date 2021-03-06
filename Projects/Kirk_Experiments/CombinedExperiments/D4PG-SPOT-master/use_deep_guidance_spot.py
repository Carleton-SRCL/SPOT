"""
This script loads in a trained policy neural network and uses it for inference.

Typically this script will be executed on the Nvidia Jetson TX2 board during an
experiment in the Spacecraft Robotics and Control Laboratory at Carleton
University.

Script created: June 12, 2019
@author: Kirk (khovell@gmail.com)
"""

import tensorflow as tf
import numpy as np
import socket
import time

from settings import Settings
from build_neural_networks import BuildActorNetwork

"""
*# Relative pose expressed in the chaser's body frame; everything else in Inertial frame #*
Deep guidance output in x and y are in the chaser body frame
"""

def make_C_bI(angle):        
    C_bI = np.array([[ np.cos(angle), np.sin(angle)],
                     [-np.sin(angle), np.cos(angle)]]) # [2, 2]        
    return C_bI

#%%
testing = True # [boolean] Set to True for testing purposes (without using the Jetson)

offset_x = 0 # amount the end-effector misses the target * In the chaser's body frame *
offset_y = 0 # amount the end-effector misses the target * In the chaser's body frame *
offset_angle = 0 # amount the end-effector misses the target


#%%
# Clear any old graph
tf.reset_default_graph()
counter = 1

# Initialize Tensorflow, and load in policy
with tf.Session() as sess:
    # Building the policy network
    state_placeholder = tf.placeholder(dtype = tf.float32, shape = [None, Settings.OBSERVATION_SIZE], name = "state_placeholder")
    actor = BuildActorNetwork(state_placeholder, scope='learner_actor_main')

    # Loading in trained network weights
    print("Attempting to load in previously-trained model\n")
    saver = tf.train.Saver() # initialize the tensorflow Saver()

    # Try to load in policy network parameters
    try:
        ckpt = tf.train.get_checkpoint_state('../')
        saver.restore(sess, ckpt.model_checkpoint_path)
        print("\nModel successfully loaded!\n")

    except (ValueError, AttributeError):
        print("No model found... quitting :(")
        raise SystemExit

    ##################################################
    #### Start communication with JetsonRepeater #####
    ##################################################
    # Initializing
    connected = False
    
    # Parameters for normalizing the input
    relevant_state_mean = np.delete(Settings.STATE_MEAN, Settings.IRRELEVANT_STATES)
    relevant_half_range = np.delete(Settings.STATE_HALF_RANGE, Settings.IRRELEVANT_STATES)

    # Looping forever
    while True:
        if not connected and not testing: # If we aren't connected, try to connect to the JetsonRepeater program
            try: # Try to connect
                client_socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
                client_socket.connect("/tmp/jetsonRepeater") # Connecting...
                connected = True
                print("Connected to JetsonRepeater! Deep Guidance Started")
            except: # If connection attempt failed
                print("Connection to JetsonRepeater FAILED. Trying to re-connect in 1 second")
                time.sleep(1)
                continue # Restart from while True
        else: # We are connected!
            if not testing:
                try: # Try to receive data
                    client_socket.recv(4096) # Clear the buffer
                    data = client_socket.recv(4096) # Read the next value
                except: # If receive fails, we have lost communication with the JetsonRepeater
                    print("Lost communication with JetsonRepeater")
                    connected = False
                    continue # Restart from while True
                if data == False: # If it is time to end
                    print('Terminating Deep Guidance')
                    break
                else: # We got good data!
                    input_data = data.decode("utf-8")
                    #print('Got message: ' + str(data.decode("utf-8")))

            ############################################################
            ##### Received data! Process it and return the result! #####
            ############################################################

            # Receive position data
            if testing:
                input_data_array = np.array([3.5/3, 2.4/2, 0, 0,0,0, 3.5*2/3, 2.4/2, np.pi/2, 0, 0, 0])
            else:
                input_data_array = np.array(input_data.splitlines()).astype(np.float32)
                # input_data_array is: [red_x, red_y, red_theta, red_vx, red_vy, red_omega, black_x, black_y, black_theta, black_vx, black_vy, black_omega]                
            red_x, red_y, red_theta, red_vx, red_vy, red_omega, black_x, black_y, black_theta, black_vx, black_vy, black_omega = input_data_array
                
            ##############################################################
            ### Receive relative pose information from Computer vision ###
            ##############################################################
            #TODO: Receive these things from Frank
            frank_sees_target = False
            frank_relative_x = 0
            frank_relative_y = 0
            frank_relative_angle = 0            
                        

            #################################
            ### Building the Policy Input ###
            #################################            
            # Total state is [relative_x, relative_y, relative_vx, relative_vy, relative_angle, relative_angular_velocity, chaser_x, chaser_y, chaser_theta, target_x, target_y, target_theta, chaser_vx, chaser_vy, chaser_omega, target_vx, target_vy, target_omega] *# Relative pose expressed in the chaser's body frame; everythign else in Inertial frame #*
            # State input: [relative_x, relative_y, relative_angle, chaser_theta, chaser_vx, chaser_vy, chaser_omega, target_omega]
            # Also normalize it properly
            
            if frank_sees_target:
                policy_input = np.array([frank_relative_x, frank_relative_y, frank_relative_angle - offset_angle, red_theta, red_vx, red_vy, red_omega, black_omega])
            else:
                # Calculating the relative X and Y in the chaser's body frame
                relative_pose_inertial = np.array([black_x - red_x - offset_x, black_y - red_y - offset_y])
                relative_pose_body = np.matmul(make_C_bI(red_theta), relative_pose_inertial)
                policy_input = np.array([relative_pose_body[0], relative_pose_body[1], (black_theta - red_theta - offset_angle)%(2*np.pi), red_theta, red_vx, red_vy, red_omega, black_omega])

            # Normalizing            
            if Settings.NORMALIZE_STATE:
                normalized_policy_input = (policy_input - relevant_state_mean)/relevant_half_range
            else:
                normalized_policy_input = policy_input
                
            # Reshaping the input
            normalized_policy_input = normalized_policy_input.reshape([-1, Settings.OBSERVATION_SIZE])

            # Run processed state through the policy
            deep_guidance = sess.run(actor.action_scaled, feed_dict={state_placeholder:normalized_policy_input})[0] # [accel_x, accel_y, alpha]
            
            #################################################################
            ### Cap output if we are exceeding the max allowable velocity ###
            #################################################################
            # Checking whether our velocity is too large AND the acceleration is trying to increase said velocity... in which case we set the desired_linear_acceleration to zero.
            current_velocity = np.array([red_vx, red_vy, red_omega])
            deep_guidance[(np.abs(current_velocity) > Settings.VELOCITY_LIMIT) & (np.sign(deep_guidance) == np.sign(current_velocity))] = 0  

            # Return commanded action to the Raspberry Pi 3
            if testing:
                pass
            else:
                out_data = str(deep_guidance[0]) + "\n" + str(deep_guidance[1]) + "\n" + str(deep_guidance[2]) + "\n"
                client_socket.send(out_data.encode())
            
            if counter % 2000 == 0:
                #print("Input from Pi: ", input_data_array)
                #print("Policy input: ", policy_input)
                #print("Normalized policy input: ", normalized_policy_input)
                print("Output to Pi: ", deep_guidance, " In RED body frame")
                print(normalized_policy_input)
            # Incrementing the counter
            counter = counter + 1

print('Done :)')
