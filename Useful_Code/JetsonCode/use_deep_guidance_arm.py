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
import threading
from collections import deque

# import code # for debugging
#code.interact(local=dict(globals(), **locals())) # Ctrl+D or Ctrl+Z to continue execution

try:
    from settings import Settings
except:
    print("You must load the 'manipulator' environment in settings\n\nQuitting.")
    raise SystemExit
from build_neural_networks import BuildActorNetwork

assert Settings.ENVIRONMENT == 'manipulator'

# Load an environment to use methods from
environment_file = __import__('environment_' + Settings.ENVIRONMENT) # importing the environment

"""
*# Relative pose expressed in the chaser's body frame; everything else in Inertial frame #*
Deep guidance output in x and y are in the chaser body frame
"""

# Are we testing?
testing = False

CHECK_VELOCITY_LIMITS_IN_PYTHON = True
HARD_CODE_TARGET_SPIN = False
TARGET_SPIN_VALUE = -7*np.pi/180 # [rad/s]
SUCCESSFUL_DOCKING_RADIUS = 0.04 # [m] [default: 0.04] overwrite the successful docking radius defined in the environment



###############################
### User-defined parameters ###
###############################
offset_x = 0 # Position offset of the target in its body frame
offset_y = 0 # Position offset of the target in its body frame
offset_angle = 0 # Angle offset of the target in its body frame

# Do you want to debug with constant accelerations?
DEBUG_CONTROLLER_WITH_CONSTANT_ACCELERATIONS = False
constant_Ax = 0 # [m/s^2] in inertial frame
constant_Ay = 0 # [m/s^2] in inertial frame
constant_alpha = 0 # [rad/s^2] in inertial frame
constant_alpha_shoulder = 0 # [rad/s^2]
constant_alpha_elbow = 0# [rad/s^2]
constant_alpha_wrist = 0# [rad/s^2]

def make_C_bI(angle):        
    C_bI = np.array([[ np.cos(angle), np.sin(angle)],
                     [-np.sin(angle), np.cos(angle)]]) # [2, 2]        
    return C_bI



class MessageParser:
    
    def __init__(self, testing, client_socket, messages_to_deep_guidance, stop_run_flag):
        
        print("Initializing Message Parser!")
        self.client_socket = client_socket
        self.messages_to_deep_guidance = messages_to_deep_guidance
        self.stop_run_flag = stop_run_flag
        self.testing = testing

        
        # Items from the Pi
        self.Pi_time = 0
        self.Pi_red_x = 0
        self.Pi_red_y = 0
        self.Pi_red_theta = 0
        self.Pi_red_Vx = 0
        self.Pi_red_Vy = 0
        self.Pi_red_omega = 0
        self.Pi_black_x = 0
        self.Pi_black_y = 0
        self.Pi_black_theta = 0
        self.Pi_black_Vx = 0
        self.Pi_black_Vy = 0
        self.Pi_black_omega = 0
        
        self.shoulder_theta = 0
        self.elbow_theta = 0
        self.wrist_theta = 0
        self.shoulder_omega = 0
        self.elbow_omega = 0
        self.wrist_omega = 0
        print("Done initializing parser!")
        
        
    def run(self):
        
        print("Running Message Parser!")
        
        # Run until we want to stop
        while not self.stop_run_flag.is_set():
            
            if self.testing:
                # Assign test values
                
                # Items from the Pi
                self.Pi_time = 15
                self.Pi_red_x = 3
                self.Pi_red_y = 1
                self.Pi_red_theta = 0.5
                self.Pi_red_Vx = 0
                self.Pi_red_Vy = 0
                self.Pi_red_omega = 0
                self.Pi_black_x = 1
                self.Pi_black_y = 1
                self.Pi_black_theta = 3.1
                self.Pi_black_Vx = 0
                self.Pi_black_Vy = 0
                self.Pi_black_omega = 0
                
                self.shoulder_theta = 1
                self.elbow_theta = 1.2
                self.wrist_theta = 0.5
                self.shoulder_omega = 0
                self.elbow_omega = 0
                self.wrist_omega = 0
            else:
                # It's real
                try:
                    data = self.client_socket.recv(4096) # Read the next value
                except socket.timeout:
                    print("Socket timeout")
                    continue
                data_packet = np.array(data.decode("utf-8").splitlines())
                #print('Got message: ' + str(data.decode("utf-8")))

                # We received a packet from the Pi
                # input_data_array is: [time, red_x, red_y, red_angle, red_vx, red_vy, red_dangle, black_x, black_y, black_angle, black_vx, black_vy, black_dangle, shoulder_angle, elbow_angle, wrist_angle, shoulder_omega, elbow_omega, wrist_omega]  
                try:
                    self.Pi_time, self.Pi_red_x, self.Pi_red_y, self.Pi_red_theta, self.Pi_red_Vx, self.Pi_red_Vy, self.Pi_red_omega, self.Pi_black_x, self.Pi_black_y, self.Pi_black_theta, self.Pi_black_Vx, self.Pi_black_Vy, self.Pi_black_omega, self.shoulder_theta, self.elbow_theta, self.wrist_theta, self.shoulder_omega, self.elbow_omega, self.wrist_omega = data_packet.astype(np.float32)
                except:
                    print("Failed data read from jetsonRepeater.py, continuing...")
                    continue
                
                if HARD_CODE_TARGET_SPIN:
                    self.Pi_black_omega = TARGET_SPIN_VALUE
                
                # Apply the offsets to the target
                offsets_target_body = np.array([offset_x, offset_y])
                offsets_target_inertial = np.matmul(make_C_bI(self.Pi_black_theta).T, offsets_target_body)
                self.Pi_black_x = self.Pi_black_x - offsets_target_inertial[0]
                self.Pi_black_y = self.Pi_black_y - offsets_target_inertial[1]
                self.Pi_black_theta = self.Pi_black_theta - offset_angle                                
                
            # Write the data to the queue for DeepGuidanceModelRunner to use!
            """ This queue is thread-safe. If I append multiple times without popping, the data in the queue is overwritten. Perfect! """
            #(self.Pi_time, self.Pi_red_x, self.Pi_red_y, self.Pi_red_theta, self.Pi_red_Vx, self.Pi_red_Vy, self.Pi_red_omega, self.Pi_black_x, self.Pi_black_y, self.Pi_black_theta, self.Pi_black_Vx, self.Pi_black_Vy, self.Pi_black_omega, self.shoulder_theta, self.elbow_theta, self.wrist_theta, self.shoulder_omega, self.elbow_omega, self.wrist_omega)
            self.messages_to_deep_guidance.append((self.Pi_time, self.Pi_red_x, self.Pi_red_y, self.Pi_red_theta, self.Pi_red_Vx, self.Pi_red_Vy, self.Pi_red_omega, self.Pi_black_x, self.Pi_black_y, self.Pi_black_theta, self.Pi_black_Vx, self.Pi_black_Vy, self.Pi_black_omega, self.shoulder_theta, self.elbow_theta, self.wrist_theta, self.shoulder_omega, self.elbow_omega, self.wrist_omega))
        
        print("Message handler gently stopped")
 

class DeepGuidanceModelRunner:
    
    def __init__(self, testing, client_socket, messages_to_deep_guidance, stop_run_flag):
        
        print("Initializing deep guidance model runner")
        self.client_socket = client_socket
        self.messages_to_deep_guidance = messages_to_deep_guidance
        self.stop_run_flag = stop_run_flag
        self.testing = testing
        
        # Initializing a variable to check if we've docked
        self.have_we_docked = 0.
                
        # Holding the previous position so we know when SPOTNet gives a new update
        self.previousSPOTNet_relative_x = 0.0
        
        # Initialize an environment so we can use its methods
        self.environment = environment_file.Environment()
        self.environment.reset(False)

        # Overwrite the successful docking radius
        self.environment.SUCCESSFUL_DOCKING_RADIUS = SUCCESSFUL_DOCKING_RADIUS
        
        
        # Uncomment this on TF2.0
        #tf.compat.v1.disable_eager_execution()
        
        # Clear any old graph
        tf.reset_default_graph()
        
        # Initialize Tensorflow, and load in policy
        self.sess = tf.Session()
        # Building the policy network
        self.state_placeholder = tf.placeholder(dtype = tf.float32, shape = [None, Settings.OBSERVATION_SIZE], name = "state_placeholder")
        self.actor = BuildActorNetwork(self.state_placeholder, scope='learner_actor_main')
    
        # Loading in trained network weights
        print("Attempting to load in previously-trained model\n")
        saver = tf.train.Saver() # initialize the tensorflow Saver()
    
        # Try to load in policy network parameters
        try:
            ckpt = tf.train.get_checkpoint_state('../')
            saver.restore(self.sess, ckpt.model_checkpoint_path)
            print("\nModel successfully loaded!\n")
    
        except (ValueError, AttributeError):
            print("Model: ", ckpt.model_checkpoint_path, " not found... :(")
            raise SystemExit
        
        print("Done initializing model!")

    def run(self):
        
        print("Running Deep Guidance!")
        
        counter = 1
        # Parameters for normalizing the input
        relevant_state_mean = np.delete(Settings.STATE_MEAN, Settings.IRRELEVANT_STATES)
        relevant_half_range = np.delete(Settings.STATE_HALF_RANGE, Settings.IRRELEVANT_STATES)
        
        # To log data
        data_log = []
        
        # Run zeros through the policy to ensure all libraries are properly loaded in
        deep_guidance = self.sess.run(self.actor.action_scaled, feed_dict={self.state_placeholder:np.zeros([1, Settings.OBSERVATION_SIZE])})[0]            
        
        # Run until we want to stop
        while not stop_run_flag.is_set():            
                       
            # Total state is [relative_x, relative_y, relative_vx, relative_vy, relative_angle, relative_angular_velocity, chaser_x, chaser_y, chaser_theta, target_x, target_y, target_theta, chaser_vx, chaser_vy, chaser_omega, target_vx, target_vy, target_omega] *# Relative pose expressed in the chaser's body frame; everything else in Inertial frame #*
            # Network input: [relative_x, relative_y, relative_angle, chaser_theta, chaser_vx, chaser_vy, chaser_omega, target_omega] ** Normalize it first **
            
            # Get data from Message Parser
            try:
                Pi_time, Pi_red_x, Pi_red_y, Pi_red_theta, \
                Pi_red_Vx, Pi_red_Vy, Pi_red_omega,        \
                Pi_black_x, Pi_black_y, Pi_black_theta,    \
                Pi_black_Vx, Pi_black_Vy, Pi_black_omega,  \
                shoulder_theta, elbow_theta, wrist_theta, \
                shoulder_omega, elbow_omega, wrist_omega = self.messages_to_deep_guidance.pop()
            except IndexError:
                # Queue was empty, try agian
                continue
                       
            #############################
            ### Check if we've docked ###
            #############################
            # Check the reward function based off this state
            self.environment.chaser_position   = np.array([Pi_red_x, Pi_red_y, Pi_red_theta])
            self.environment.chaser_velocity   = np.array([Pi_red_Vx, Pi_red_Vy, Pi_red_omega])
            self.environment.target_position   = np.array([Pi_black_x, Pi_black_y, Pi_black_theta])
            self.environment.target_velocity   = np.array([Pi_black_Vx, Pi_black_Vy, Pi_black_omega])
            self.environment.arm_angles        = np.array([shoulder_theta, elbow_theta, wrist_theta])
            self.environment.arm_angular_rates = np.array([shoulder_omega, elbow_omega, wrist_omega])
            
            # Get environment to check for collisions
            self.environment.update_end_effector_and_docking_locations()
            self.environment.update_end_effector_location_body_frame()
            self.environment.update_relative_pose_body_frame()
            self.environment.check_collisions()
            
            # Ask the environment whether docking occurred
            self.have_we_docked = np.max([self.have_we_docked, float(self.environment.docked)])
            
            # Extracting end-effector position and docking port position in the Inertial frame
            end_effector_position = self.environment.end_effector_position
            docking_port_position = self.environment.docking_port_position
            
            # Calculating relative position between the docking port and the end-effector in the Target's body frame
            docking_error_inertial = end_effector_position - docking_port_position
            docking_error_target_body = np.matmul(make_C_bI(Pi_black_theta), docking_error_inertial)
            print("Distance from cone to end-effector in target body frame: ", docking_error_target_body, " Environment thinks we've docked: ", self.have_we_docked)
            
            
            #################################
            ### Building the Policy Input ###
            ################################# 
            total_state = self.environment.make_total_state()
            policy_input = np.delete(total_state, Settings.IRRELEVANT_STATES)
            
            # Normalizing            
            if Settings.NORMALIZE_STATE:
                normalized_policy_input = (policy_input - relevant_state_mean)/relevant_half_range
            else:
                normalized_policy_input = policy_input
                
            # Reshaping the input
            normalized_policy_input = normalized_policy_input.reshape([-1, Settings.OBSERVATION_SIZE])
    
            # Run processed state through the policy
            deep_guidance = self.sess.run(self.actor.action_scaled, feed_dict={self.state_placeholder:normalized_policy_input})[0] # [accel_x, accel_y, alpha]
            
            # Rotating the command into the inertial frame
            if not Settings.ACTIONS_IN_INERTIAL:
                deep_guidance[0:2] = np.matmul(make_C_bI(Pi_red_theta).T,deep_guidance[0:2])
                     
            # Commanding constant values in the inertial frame for testing purposes
            if DEBUG_CONTROLLER_WITH_CONSTANT_ACCELERATIONS:                
                deep_guidance[0] = constant_Ax # [m/s^2]
                deep_guidance[1] = constant_Ay # [m/s^2]
                deep_guidance[2] = constant_alpha # [rad/s^2]
                deep_guidance[3] = constant_alpha_shoulder # [rad/s^2]
                deep_guidance[4] = constant_alpha_elbow # [rad/s^2]]
                deep_guidance[5] = constant_alpha_wrist # [rad/s^2]
													  
            #################################################################
            ### Cap output if we are exceeding the max allowable velocity ###
            #################################################################
            # Stopping the command of additional velocity when we are already at our maximum
            """ The check for arm velocity exceeding has been transferred to Simulink - June 1, 2021 """
            if CHECK_VELOCITY_LIMITS_IN_PYTHON:                    
                current_velocity = np.array([Pi_red_Vx, Pi_red_Vy, Pi_red_omega])               
                deep_guidance[:len(current_velocity)][(np.abs(current_velocity) > Settings.VELOCITY_LIMIT[:len(current_velocity)]) & (np.sign(deep_guidance[:len(current_velocity)]) == np.sign(current_velocity))] = 0
                        
            # Return commanded action to the Raspberry Pi 3
            if self.testing:
                print(deep_guidance)                
            
            else:
                deep_guidance_acceleration_signal_to_pi = str(deep_guidance[0]) + "\n" + str(deep_guidance[1]) + "\n" + str(deep_guidance[2]) + "\n" + str(deep_guidance[3]) + "\n" + str(deep_guidance[4]) + "\n" + str(deep_guidance[5]) + "\n" + str(self.have_we_docked) + "\n" 
                self.client_socket.send(deep_guidance_acceleration_signal_to_pi.encode())
            
            if counter % 2000 == 0:
                print("Output to Pi: ", deep_guidance, " In table inertial frame or joint frame")
                print(normalized_policy_input)
            # Incrementing the counter
            counter = counter + 1
            
            # Log this timestep's data only if the experiment has actually started
            if Pi_time > 0:                
                data_log.append([Pi_time, deep_guidance[0], deep_guidance[1], deep_guidance[2], \
                                 deep_guidance[3], deep_guidance[4], deep_guidance[5], \
                                 Pi_red_x, Pi_red_y, Pi_red_theta, \
                                 Pi_red_Vx, Pi_red_Vy, Pi_red_omega,        \
                                 Pi_black_x, Pi_black_y, Pi_black_theta,    \
                                 Pi_black_Vx, Pi_black_Vy, Pi_black_omega,  \
                                 shoulder_theta, elbow_theta, wrist_theta, \
                                 shoulder_omega, elbow_omega, wrist_omega, self.have_we_docked])
        
        print("Model gently stopped.")
        
        if len(data_log) > 0: 
            print("Saving data to file...",end='')               
            with open('deep_guidance_data_' + time.strftime('%Y-%m-%d_%H-%M-%S', time.localtime()) + '.txt', 'wb') as f:
                    np.save(f, np.asarray(data_log))
        else:
            print("Not saving a log because there is no data to write")
                
        print("Done!")
        # Close tensorflow session
        self.sess.close()


##################################################
#### Start communication with JetsonRepeater #####
##################################################
if testing:
    client_socket = 0
else:
    # Looping forever until we are connected
    while True:
        try: # Try to connect
            client_socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            client_socket.connect("/tmp/jetsonRepeater") # Connecting...
            client_socket.settimeout(2) # Setting the socket timeout to 2 seconds
            print("Connected to JetsonRepeater!")
            break
        except: # If connection attempt failed
            print("Connection to JetsonRepeater FAILED. Trying to re-connect in 1 second")
            time.sleep(1)
    # WE ARE CONNECTED 

# Generate Queues
messages_to_deep_guidance = deque(maxlen = 1)

#####################
### START THREADS ###
#####################
all_threads = []
stop_run_flag = threading.Event() # Flag to stop all threads 
# Initialize Message Parser
message_parser = MessageParser(testing, client_socket, messages_to_deep_guidance, stop_run_flag)
# Initialize Deep Guidance Model
deep_guidance_model = DeepGuidanceModelRunner(testing, client_socket, messages_to_deep_guidance, stop_run_flag)
       
all_threads.append(threading.Thread(target = message_parser.run))
all_threads.append(threading.Thread(target = deep_guidance_model.run))

#############################################
##### STARTING EXECUTION OF ALL THREADS #####
#############################################
#                                           #
#                                           #
for each_thread in all_threads:             #
#                                           #
    each_thread.start()                     #
#                                           #
#                                           #
#############################################
############## THREADS STARTED ##############
#############################################
counter_2 = 1   
try:       
    while True:
        time.sleep(0.5)
        if counter_2 % 200 == 0:
            print("100 seconds in, trying to stop gracefully")
            stop_run_flag.set()
            for each_thread in all_threads:
                each_thread.join()
            break
except KeyboardInterrupt:
    print("Interrupted by user. Ending gently")
    stop_run_flag.set()
    for each_thread in all_threads:
            each_thread.join()

        

print('Done :)')
