
"""
This script provides the environment for a docking experiment in the SPOT
facility at Carleton University.

All dynamic environments I create will have a standardized architecture. The
reason for this is I have one learning algorithm and many environments. All
environments are responsible for:
    - dynamics propagation (via the step method)
    - initial conditions   (via the reset method)
    - reporting environment properties (defined in __init__)
    - seeding the dynamics (via the seed method)
    - animating the motion (via the render method):
        - Rendering is done all in one shot by passing the completed states
          from a trial to the render() method.

Outputs:
    Reward must be of shape ()
    State must be of shape (OBSERVATION_SIZE,)
    Done must be a bool

Inputs:
    Action input is of shape (ACTION_SIZE,)

Communication with agent:
    The agent communicates to the environment through two queues:
        agent_to_env: the agent passes actions or reset signals to the environment
        env_to_agent: the environment returns information to the agent

Reward system:
        - Zero reward at all timesteps except when docking is achieved
        - A large reward when docking occurs. The episode also terminates when docking occurs
        - A variety of penalties to help with docking, such as:
            - penalty for end-effector angle (so it goes into the docking cone properly)
            - penalty for relative velocity during the docking (so the end-effector doesn't jab the docking cone)
        - A penalty for colliding with the target

State clarity:
    - self.dynamic_state contains the chaser states propagated in the dynamics
    - self.observation is passed to the agent and is a combination of the dynamic
                       state and the target position
    - self.OBSERVATION_SIZE 


Started November 5, 2020
@author: Kirk Hovell (khovell@gmail.com)
"""
import numpy as np
import os
import signal
import multiprocessing
import queue
from scipy.integrate import odeint # Numerical integrator

import matplotlib
matplotlib.use('Agg')

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import matplotlib.gridspec as gridspec

from shapely.geometry import Point, Polygon # for collision detection

class Environment:

    def __init__(self):
        ##################################
        ##### Environment Properties #####
        ##################################
        self.TOTAL_STATE_SIZE         = 18 # [relative_x, relative_y, relative_vx, relative_vy, relative_angle, relative_angular_velocity, chaser_x, chaser_y, chaser_theta, target_x, target_y, target_theta, chaser_vx, chaser_vy, chaser_omega, target_vx, target_vy, target_omega] *# Relative pose expressed in the chaser's body frame; everythign else in Inertial frame #*
        ### Note: TOTAL_STATE contains all relevant information describing the problem, and all the information needed to animate the motion
        #         TOTAL_STATE is returned from the environment to the agent.
        #         A subset of the TOTAL_STATE, called the 'observation', is passed to the policy network to calculate acitons. This takes place in the agent
        #         The TOTAL_STATE is passed to the animator below to animate the motion.
        #         The chaser and target state are contained in the environment. They are packaged up before being returned to the agent.
        #         The total state information returned must be as commented beside self.TOTAL_STATE_SIZE.
        #self.IRRELEVANT_STATES                = [6,7,8,9,10,11,12,13,14,15,16,17] # [original] indices of states who are irrelevant to the policy network
        #self.IRRELEVANT_STATES                = [2,3,5,9,10,11] # [chaser_x, chaser_y, chaser_theta, relative_x, relative_y, relative_angle, chaser_vx, chaser_vy, chaser_omega, target_vx, target_vy, target_omega]
        #self.IRRELEVANT_STATES                = [2,3,5,6,7,9,10,11] # [chaser_theta, relative_x, relative_y, relative_angle, chaser_vx, chaser_vy, chaser_omega, target_vx, target_vy, target_omega] # omitting chaser_x, chaser_y
        self.IRRELEVANT_STATES                = [2,3,5,6,7,9,10,11,15,16] # [relative_x, relative_y, relative_angle, chaser_theta, chaser_vx, chaser_vy, chaser_omega, target_omega]; [chaser_theta, relative_x, relative_y, relative_angle, chaser_vx, chaser_vy, chaser_omega, target_omega] # omitting chaser_x, chaser_y, target_vx, target_vy
        self.OBSERVATION_SIZE                 = self.TOTAL_STATE_SIZE - len(self.IRRELEVANT_STATES) # the size of the observation input to the policy
        self.ACTION_SIZE                      = 3 # [x_dot_dot, y_dot_dot, theta_dot_dot] in the BODY frame
        self.MAX_VELOCITY                     = 0.2 # [m/s]
        self.MAX_ANGULAR_VELOCITY             = np.pi/6 # [rad/s]
        self.LOWER_ACTION_BOUND               = np.array([-0.02, -0.02, -0.1]) # [m/s^2, m/s^2, rad/s^2]
        self.UPPER_ACTION_BOUND               = np.array([ 0.02,  0.02,  0.1]) # [m/s^2, m/s^2, rad/s^2]
        self.LOWER_STATE_BOUND                = np.array([-3., -3., -self.MAX_VELOCITY, -self.MAX_VELOCITY, -2*np.pi, -2*self.MAX_ANGULAR_VELOCITY, 0. , 0. , -6*np.pi, 0. , 0. , -6*np.pi, -self.MAX_VELOCITY, -self.MAX_VELOCITY, -self.MAX_ANGULAR_VELOCITY, -self.MAX_VELOCITY, -self.MAX_VELOCITY, -self.MAX_ANGULAR_VELOCITY]) # [m, m, m/s, m/s, rad, rad/s, m, m, rad, m, m, rad, m/s, m/s, rad/s, m/s, m/s, rad/s] // lower bound for each element of TOTAL_STATE
        self.UPPER_STATE_BOUND                = np.array([ 3.,  3.,  self.MAX_VELOCITY,  self.MAX_VELOCITY,  2*np.pi,  2*self.MAX_ANGULAR_VELOCITY, 3.5, 2.4,  6*np.pi, 3.5, 2.4,  6*np.pi,  self.MAX_VELOCITY,  self.MAX_VELOCITY,  self.MAX_ANGULAR_VELOCITY,  self.MAX_VELOCITY,  self.MAX_VELOCITY,  self.MAX_ANGULAR_VELOCITY]) # [m, m, m,s, m,s, rad, rad/s, m, m, rad, m, m, rad, m/s, m/s, rad/s, m/s, m/s, rad/s] // upper bound for each element of TOTAL_STATE
        self.INITIAL_CHASER_POSITION          = np.array([3.5/2, 1.2, 0.0]) # [m, m, rad]
        self.INITIAL_CHASER_VELOCITY          = np.array([0.0, 0.0, 0.0]) # [m/s, m/s, rad/s]
        self.INITIAL_TARGET_POSITION          = np.array([3.5/2, 1.2, 0.0]) # [m, m, rad]
        self.INITIAL_TARGET_VELOCITY          = np.array([0.0, 0.0, 0.0]) # [m/s, m/s, rad/s]
        self.NORMALIZE_STATE                  = True # Normalize state on each timestep to avoid vanishing gradients
        self.RANDOMIZE                        = True # whether or not to RANDOMIZE the state & target location
        self.RANDOMIZATION_LENGTH_X           = 3.5/2-0.2 # [m] half-range uniform randomization X position
        self.RANDOMIZATION_LENGTH_Y           = 2.4/2-0.2 # [m] half-range uniform randomization Y position
        self.RANDOMIZATION_ANGLE              = np.pi # [rad] half-range uniform randomization chaser and target angle
        self.RANDOMIZATION_TARGET_VELOCITY    = 0.0 # [m/s] half-range uniform randomization target velocity
        self.RANDOMIZATION_TARGET_OMEGA       = 2*np.pi/30 # [rad/s] half-range uniform randomization target omega
        self.MIN_V                            = -100.
        self.MAX_V                            =  125.
        self.N_STEP_RETURN                    =   5
        self.DISCOUNT_FACTOR                  =   0.95**(1/self.N_STEP_RETURN)
        self.TIMESTEP                         =   0.2 # [s]
        self.DYNAMICS_DELAY                   =   0 # [timesteps of delay] how many timesteps between when an action is commanded and when it is realized
        self.AUGMENT_STATE_WITH_ACTION_LENGTH =   0 # [timesteps] how many timesteps of previous actions should be included in the state. This helps with making good decisions among delayed dynamics.
        self.MAX_NUMBER_OF_TIMESTEPS          = 300#150 # per episode
        self.ADDITIONAL_VALUE_INFO            = False # whether or not to include additional reward and value distribution information on the animations
        self.SKIP_FAILED_ANIMATIONS           = True # Error the program or skip when animations fail?

        # Physical properties
        self.LENGTH                        = 0.3  # [m] side length
        #self.MASS                          = 10.0   # [kg] for chaser
        #self.INERTIA                       = 1/12*self.MASS*(self.LENGTH**2 + self.LENGTH**2) # 0.15 [kg m^2]
        #self.DOCKING_PORT_MOUNT_POSITION   = np.array([0, self.LENGTH/2]) # position of the docking cone on the target in its body frame
        #self.DOCKING_PORT_CORNER1_POSITION = self.DOCKING_PORT_MOUNT_POSITION + [ 0.05, 0.1] # position of the docking cone on the target in its body frame
        #self.DOCKING_PORT_CORNER2_POSITION = self.DOCKING_PORT_MOUNT_POSITION + [-0.05, 0.1] # position of the docking cone on the target in its body frame
        
        self.DOCKING_PORT_MOUNT_POSITION = np.array([0.06944, 0.21007]) # [m] with respect to the centre of mass
        self.DOCKING_PORT_CORNER1_POSITION = self.DOCKING_PORT_MOUNT_POSITION + [ 0.0508, 0.0432562] # position of the docking cone on the target in its body frame
        self.DOCKING_PORT_CORNER2_POSITION = self.DOCKING_PORT_MOUNT_POSITION + [-0.0508, 0.0432562] # position of the docking cone on the target in its body frame
        
        self.PHI      = -24.34*np.pi/180 # [rad] angle of anchor point of arm with respect to spacecraft body frame
        self.B0       = 0.2586 # scalar distance from centre of mass to arm attachment point
        self.THETA_1  = 1.053 # [rad] optimal final orientation of joint 1
        self.THETA_2  = -1.36 # [rad] optimal final orientation of joint 2
        self.THETA_3  =  0.3068 # [rad] optimal final orientation of joint 3
        self.MASS_C   = 17.6429  # [kg] for chaser
        self.M1       = 0.3377 # [kg] link mass
        self.M2       = 0.3281 # [kg] link mass
        self.M3       = 0.0111 # [kg] link mass
        self.MASS     = self.MASS_C + self.M1 + self.M2 + self.M3 # [kg] total mass
        self.A1       = 0.1933 # [m] base of link to centre of mass
        self.B1       = 0.1117 # [m] centre of mass to end of link
        self.A2       = 0.1993 # [m] base of link to centre of mass
        self.B2       = 0.1057 # [m] centre of mass to end of link
        self.A3       = 0.0621 # [m] base of link to centre of mass
        self.B3       = 0.0159 # [m] centre of mass to end of link
        self.INERTIA_C = 2.873E-1 # [kg m^2] base inertia
        self.INERTIA1 = 3.75E-3 # [kg m^2] link inertia
        self.INERTIA2 = 3.413E-3 # [kg m^2] link inertia
        self.INERTIA3 = 5.64E-5 # [kg m^2] link inertia

        # Fixed optimal arm orientation
        self.SHOULDER_POSITION = self.B0*np.array([np.cos(self.PHI), np.sin(self.PHI)]) # [m] position of the arm mounting point on the chaser in the body frame
        self.ELBOW_POSITION = self.SHOULDER_POSITION + (self.A1 + self.B1)*np.array([np.cos(self.THETA_1), np.sin(self.THETA_1)])
        self.WRIST_POSITION = self.ELBOW_POSITION + (self.A2 + self.B2)*np.array([np.cos(self.THETA_1 + self.THETA_2), np.sin(self.THETA_1 + self.THETA_2)])
        self.END_EFFECTOR_POSITION = self.WRIST_POSITION + (self.A3 + self.B3)*np.array([np.cos(self.THETA_1 + self.THETA_2 + self.THETA_3), np.sin(self.THETA_1 + self.THETA_2 + self.THETA_3)])
        self.ARM_MOUNT_POSITION = self.SHOULDER_POSITION - np.array([0.08,0])
        
        # Calculate combined inertia
        self.R1 = self.SHOULDER_POSITION + (self.A1)*np.array([np.cos(self.THETA_1), np.sin(self.THETA_1)])
        self.R2 = self.ELBOW_POSITION + (self.A2)*np.array([np.cos(self.THETA_1 + self.THETA_2), np.sin(self.THETA_1 + self.THETA_2)])
        self.R3 = self.WRIST_POSITION + (self.A3)*np.array([np.cos(self.THETA_1 + self.THETA_2 + self.THETA_3), np.sin(self.THETA_1 + self.THETA_2 + self.THETA_3)])
        self.INERTIA = self.INERTIA_C + self.INERTIA1 + self.M1*np.linalg.norm(self.R1)**2 + self.INERTIA2 + self.M2*np.linalg.norm(self.R2)**2 + self.INERTIA3 + self.M3*np.linalg.norm(self.R3)**2
        
        # self.ARM_MOUNT_POSITION            = np.array([0, self.LENGTH/2]) # [m] position of the arm mounting point on the chaser in the body frame
        # self.SHOULDER_POSITION             = self.ARM_MOUNT_POSITION + [0, 0.05] # [m] position of the arm's shoulder in the chaser body frame
        # self.ELBOW_POSITION                = self.SHOULDER_POSITION + [0.3*np.sin(np.pi/6), 0.3*np.cos(np.pi/6)] # [m] position of the arm's elbow in the chaser body frame
        # self.WRIST_POSITION                = self.ELBOW_POSITION + [0.3*np.sin(np.pi/4),-0.3*np.cos(np.pi/4)] # [m] position of the arm's wrist in the chaser body frame
        # self.END_EFFECTOR_POSITION         = self.WRIST_POSITION + [0.1, 0] # po sition of the optimally-deployed end-effector on the chaser in the body frame
        
        # Reward function properties
        self.DOCKING_REWARD                   = 100 # A lump-sum given to the chaser when it docks
        self.SUCCESSFUL_DOCKING_RADIUS        = 0.04 # [m] distance at which the magnetic docking can occur
        self.MAX_DOCKING_ANGLE_PENALTY        = 50 # A penalty given to the chaser, upon docking, for having an angle when docking. The penalty is 0 upon perfect docking and MAX_DOCKING_ANGLE_PENALTY upon perfectly bad docking
        self.DOCKING_EE_VELOCITY_PENALTY      = 50 # A penalty given to the chaser, upon docking, for every 1 m/s end-effector collision velocity upon docking
        self.DOCKING_ANGULAR_VELOCITY_PENALTY = 25 # A penalty given to the chaser, upon docking, for every 1 rad/s angular body velocity upon docking
        self.END_ON_FALL                      = True # end episode on a fall off the table        
        self.FALL_OFF_TABLE_PENALTY           = 100.
        self.CHECK_CHASER_TARGET_COLLISION    = True
        self.TARGET_COLLISION_PENALTY         = 5 # [rewards/timestep] penalty given for colliding with target  
        self.CHECK_END_EFFECTOR_COLLISION     = True # Whether to do collision detection on the end-effector
        self.CHECK_END_EFFECTOR_FORBIDDEN     = True # Whether to expand the collision area to include the forbidden zone
        self.END_EFFECTOR_COLLISION_PENALTY   = 5 # [rewards/timestep] Penalty for end-effector collisions (with target or optionally with the forbidden zone)
        self.END_ON_COLLISION                 = True # Whether to end the episode upon a collision to prevent collisions more drastically.
        self.GIVE_MID_WAY_REWARD              = True # Whether or not to give a reward mid-way towards the docking port to encourage the learning to move in the proper direction
        self.MID_WAY_REWARD_RADIUS            = 0.1 # [ms] the radius from the DOCKING_PORT_MOUNT_POSITION that the mid-way reward is given
        self.MID_WAY_REWARD                   = 25 # The value of the mid-way reward
        
        # Test time properties
        self.TEST_ON_DYNAMICS            = True # Whether or not to use full dynamics along with a PD controller at test time
        self.KINEMATIC_NOISE             = False # Whether or not to apply noise to the kinematics in order to simulate a poor controller
        self.KINEMATIC_POSITION_NOISE_SD = [0.2, 0.2, 0.2] # The standard deviation of the noise that is to be applied to each position element in the state
        self.KINEMATIC_VELOCITY_NOISE_SD = [0.1, 0.1, 0.1] # The standard deviation of the noise that is to be applied to each velocity element in the state
        self.FORCE_NOISE_AT_TEST_TIME    = False # [Default -> False] Whether or not to force kinematic noise to be present at test time
        self.KI                          = [18.3, 18.3, 0.45] # Integral gain for the integral-linear acceleration controller in [X, Y, and angle] (how fast does the commanded acceleration get realized)
        
        
        # Some calculations that don't need to be changed
        self.VELOCITY_LIMIT           = np.array([self.MAX_VELOCITY, self.MAX_VELOCITY, self.MAX_ANGULAR_VELOCITY]) # [m/s, m/s, rad/s] maximum allowable velocity/angular velocity; a hard cap is enforced if this velocity is exceeded in kinematics & the controller enforces the limit in dynamics & experiment
        self.LOWER_STATE_BOUND        = np.concatenate([self.LOWER_STATE_BOUND, np.tile(self.LOWER_ACTION_BOUND, self.AUGMENT_STATE_WITH_ACTION_LENGTH)]) # lower bound for each element of TOTAL_STATE
        self.UPPER_STATE_BOUND        = np.concatenate([self.UPPER_STATE_BOUND, np.tile(self.UPPER_ACTION_BOUND, self.AUGMENT_STATE_WITH_ACTION_LENGTH)]) # upper bound for each element of TOTAL_STATE        
        self.OBSERVATION_SIZE         = self.TOTAL_STATE_SIZE - len(self.IRRELEVANT_STATES) # the size of the observation input to the policy


    ###################################
    ##### Seeding the environment #####
    ###################################
    def seed(self, seed):
        np.random.seed(seed)


    ######################################
    ##### Resettings the Environment #####
    ######################################
    def reset(self, use_dynamics, test_time):
        # This method resets the state and returns it
        """ NOTES:
               - if use_dynamics = True -> use dynamics
               - if test_time = True -> do not add "controller noise" to the kinematics
        """
        # Setting the default to be kinematics
        self.dynamics_flag = False

        # Resetting the mid-way flag
        self.not_yet_mid_way = True

        # Logging whether it is test time for this episode
        self.test_time = test_time
        

        # If we are randomizing the initial conditions and state
        if self.RANDOMIZE:
            # Randomizing initial state in Inertial frame
            self.chaser_position = self.INITIAL_CHASER_POSITION + np.random.uniform(low = -1, high = 1, size = 3)*[self.RANDOMIZATION_LENGTH_X, self.RANDOMIZATION_LENGTH_Y, self.RANDOMIZATION_ANGLE]
            # Randomizing target state in Inertial frame
            self.target_position = self.INITIAL_TARGET_POSITION + np.random.uniform(low = -1, high = 1, size = 3)*[self.RANDOMIZATION_LENGTH_X, self.RANDOMIZATION_LENGTH_Y, self.RANDOMIZATION_ANGLE]
            # Randomizing target velocity in Inertial frame
            self.target_velocity = self.INITIAL_TARGET_VELOCITY + np.random.uniform(low = -1, high = 1, size = 3)*[self.RANDOMIZATION_TARGET_VELOCITY, self.RANDOMIZATION_TARGET_VELOCITY, self.RANDOMIZATION_TARGET_OMEGA]
            

        else:
            # Constant initial state in Inertial frame
            self.chaser_position = self.INITIAL_CHASER_POSITION
            # Constant target location in Inertial frame
            self.target_position = self.INITIAL_TARGET_POSITION
            # Constant target velocity in Inertial frame
            self.target_velocity = self.INITIAL_TARGET_VELOCITY
        
        # Resetting the chaser's initial velocity
        self.chaser_velocity = self.INITIAL_CHASER_VELOCITY
        
        # Update docking component locations
        self.update_docking_locations()
        
        # Check for collisions
        self.check_collisions()
        # If we are colliding (unfairly) upon a reset, reset the environment again!
        if self.end_effector_collision or self.forbidden_area_collision or self.chaser_target_collision or self.elbow_target_collision:
            # Reset the environment again!
            self.reset(use_dynamics, test_time)
        
        

        # Initializing the previous velocity and control effort for the integral-acceleration controller
        self.previous_velocity = np.zeros(len(self.INITIAL_CHASER_VELOCITY))
        self.previous_control_effort = np.zeros(self.ACTION_SIZE)
                
        if use_dynamics:            
            self.dynamics_flag = True # for this episode, dynamics will be used

        # Resetting the time
        self.time = 0.
                
        # Resetting the action delay queue
        if self.DYNAMICS_DELAY > 0:
            self.action_delay_queue = queue.Queue(maxsize = self.DYNAMICS_DELAY + 1)
            for i in range(self.DYNAMICS_DELAY):
                self.action_delay_queue.put(np.zeros(self.ACTION_SIZE), False)


    def update_docking_locations(self):
        # Updates the position of the end-effector and the docking port in the Inertial frame
        
        # Make rotation matrices        
        C_Ib_chaser = self.make_C_bI(self.chaser_position[-1]).T
        C_Ib_target = self.make_C_bI(self.target_position[-1]).T
        
        # Position in Inertial = Body position (inertial) + C_Ib * EE position in body
        self.end_effector_position = self.chaser_position[:-1] + np.matmul(C_Ib_chaser, self.END_EFFECTOR_POSITION)
        self.docking_port_position = self.target_position[:-1] + np.matmul(C_Ib_target, self.DOCKING_PORT_MOUNT_POSITION)

        
    #####################################
    ##### Step the Dynamics forward #####
    #####################################
    def step(self, action):

        # Integrating forward one time step using the calculated action.
        # Oeint returns initial condition on first row then next TIMESTEP on the next row
        #########################################
        ##### PROPAGATE KINEMATICS/DYNAMICS #####
        #########################################
        if self.dynamics_flag:
            ############################
            #### PROPAGATE DYNAMICS ####
            ############################

            # First, calculate the control effort
            control_effort = self.controller(action)

            # Anything that needs to be sent to the dynamics integrator
            dynamics_parameters = [control_effort, self.MASS, self.INERTIA]

            # Propagate the dynamics forward one timestep
            next_states = odeint(dynamics_equations_of_motion, np.concatenate([self.chaser_position, self.chaser_velocity]), [self.time, self.time + self.TIMESTEP], args = (dynamics_parameters,), full_output = 0)

            # Saving the new state
            self.chaser_position = next_states[1,:len(self.INITIAL_CHASER_POSITION)] # extract position
            self.chaser_velocity = next_states[1,len(self.INITIAL_CHASER_POSITION):] # extract velocity

        else:

            # Parameters to be passed to the kinematics integrator
            kinematics_parameters = [action, len(self.INITIAL_CHASER_POSITION)]

            ###############################
            #### PROPAGATE KINEMATICS #####
            ###############################
            next_states = odeint(kinematics_equations_of_motion, np.concatenate([self.chaser_position, self.chaser_velocity]), [self.time, self.time + self.TIMESTEP], args = (kinematics_parameters,), full_output = 0)

            # Saving the new state
            self.chaser_position = next_states[1,:len(self.INITIAL_CHASER_POSITION)] # extract position
            self.chaser_velocity = next_states[1,len(self.INITIAL_CHASER_POSITION):] # extract velocity  
            
            # Optionally, add noise to the kinematics to simulate "controller noise"
            if self.KINEMATIC_NOISE and (not self.test_time or self.FORCE_NOISE_AT_TEST_TIME):
                 # Add some noise to the position part of the state
                 self.chaser_position += np.random.randn(len(self.chaser_position)) * self.KINEMATIC_POSITION_NOISE_SD
                 self.chaser_velocity += np.random.randn(len(self.chaser_velocity)) * self.KINEMATIC_VELOCITY_NOISE_SD
            
            # Ensuring the velocity is within the bounds
            self.chaser_velocity = np.clip(self.chaser_velocity, -self.VELOCITY_LIMIT, self.VELOCITY_LIMIT)


        # Step target's state ahead one timestep
        self.target_position += self.target_velocity * self.TIMESTEP

        # Update docking locations
        self.update_docking_locations()
        
        # Check for collisions
        self.check_collisions()
        
        # Increment the timestep
        self.time += self.TIMESTEP

        # Calculating the reward for this state-action pair
        reward = self.reward_function(action)

        # Check if this episode is done
        done = self.is_done()

        # Return the (reward, done)
        return reward, done


    def controller(self, action):
        # This function calculates the control effort based on the state and the
        # desired acceleration (action)
        
        ########################################
        ### Integral-acceleration controller ###
        ########################################
        desired_accelerations = action
        
        current_velocity = self.chaser_velocity # [v_x, v_y, omega]
        current_accelerations = (current_velocity - self.previous_velocity)/self.TIMESTEP # Approximating the current acceleration [a_x, a_y, alpha]

        # Checking whether our velocity is too large AND the acceleration is trying to increase said velocity... in which case we set the desired_linear_acceleration to zero.
        desired_accelerations[(np.abs(current_velocity) > self.VELOCITY_LIMIT) & (np.sign(desired_accelerations) == np.sign(current_velocity))] = 0        
        
        # Calculating acceleration error
        acceleration_error = desired_accelerations - current_accelerations
        
        # Integral-acceleration control
        control_effort = self.previous_control_effort + self.KI * acceleration_error

        # Saving the current velocity for the next timetsep
        self.previous_velocity = current_velocity
        
        # Saving the current control effort for the next timestep
        self.previous_control_effort = control_effort

        # [F_x, F_y, torque]
        return control_effort


    def reward_function(self, action):
        # Returns the reward for this TIMESTEP as a function of the state and action
        
        """
        Reward system:
                - Zero reward at all timesteps except when docking is achieved
                - A large reward when docking occurs. The episode also terminates when docking occurs
                - A variety of penalties to help with docking, such as:
                    - penalty for end-effector angle (so it goes into the docking cone properly)
                    - penalty for relative velocity during the docking (so the end-effector doesn't jab the docking cone)
                - A penalty for colliding with the target
         """ 
                
        # Initializing the reward
        reward = 0
        
        # Give a large reward for docking
        if self.docked:
            
            reward += self.DOCKING_REWARD
            
            # Penalize for end-effector angle
            # end-effector angle in the chaser body frame
            end_effector_angle_body = np.arctan2(self.END_EFFECTOR_POSITION[1] - self.WRIST_POSITION[1],self.END_EFFECTOR_POSITION[0] - self.WRIST_POSITION[0])
            end_effector_angle_inertial = end_effector_angle_body + self.chaser_position[-1]
            
            # Docking cone angle in the target body frame
            docking_cone_angle_body = np.arctan2(self.DOCKING_PORT_CORNER1_POSITION[1] - self.DOCKING_PORT_CORNER2_POSITION[1], self.DOCKING_PORT_CORNER1_POSITION[0] - self.DOCKING_PORT_CORNER2_POSITION[0])
            docking_cone_angle_inertial = docking_cone_angle_body + self.target_position[-1] - np.pi/2 # additional -pi/2 since we must dock perpendicular into the cone
            
            # Calculate the docking angle error
            docking_angle_error = (docking_cone_angle_inertial - end_effector_angle_inertial + np.pi) % (2*np.pi) - np.pi # wrapping to [-pi, pi] 
            
            # Penalize for any non-zero angle
            reward -= np.abs(np.sin(docking_angle_error/2)) * self.MAX_DOCKING_ANGLE_PENALTY
                        
            # Penalize for relative velocity during docking
            # Calculating the end-effector velocity; v_e = v_0 + omega x r_e/0
            end_effector_velocity = self.chaser_velocity[:-1] + self.chaser_velocity[-1] * np.matmul(self.make_C_bI(self.chaser_position[-1]).T,[-self.END_EFFECTOR_POSITION[1], self.END_EFFECTOR_POSITION[0]])
            
            # Calculating the docking cone velocity
            docking_cone_velocity = self.target_velocity[:-1] + self.target_velocity[-1] * np.matmul(self.make_C_bI(self.target_position[-1]).T,[-self.DOCKING_PORT_MOUNT_POSITION[1], self.DOCKING_PORT_MOUNT_POSITION[0]])
            
            # Calculating the docking velocity error
            docking_relative_velocity = end_effector_velocity - docking_cone_velocity
            
            # Applying the penalty
            reward -= np.linalg.norm(docking_relative_velocity) * self.DOCKING_EE_VELOCITY_PENALTY # 
            
            # Penalize for chaser angular velocity upon docking
            reward -= np.abs(self.chaser_velocity[-1] - self.target_velocity[-1]) * self.DOCKING_ANGULAR_VELOCITY_PENALTY
            
            if self.test_time:
                print("docking successful! Reward given: %.1f distance: %.3f; relative velocity: %.3f velocity penalty: %.1f; docking angle: %.2f angle penalty: %.1f; angular rate error: %.3f angular rate penalty %.1f" %(reward, np.linalg.norm(self.end_effector_position - self.docking_port_position), np.linalg.norm(docking_relative_velocity), np.linalg.norm(docking_relative_velocity) * self.DOCKING_EE_VELOCITY_PENALTY, docking_angle_error*180/np.pi, np.abs(np.sin(docking_angle_error/2)) * self.MAX_DOCKING_ANGLE_PENALTY,np.abs(self.chaser_velocity[-1] - self.target_velocity[-1]),np.abs(self.chaser_velocity[-1] - self.target_velocity[-1]) * self.DOCKING_ANGULAR_VELOCITY_PENALTY))
        
        if self.GIVE_MID_WAY_REWARD and self.not_yet_mid_way and self.mid_way:
            if self.test_time:
                print("Just passed the mid-way mark. Distance: %.3f at time %.1f" %(np.linalg.norm(self.end_effector_position - self.docking_port_position), self.time))
            self.not_yet_mid_way = False
            reward += self.MID_WAY_REWARD
            #Debug why this gets printed 4 times sometimes at the end of the episode!
        
        # Giving a penalty for colliding with the target
        if self.chaser_target_collision:
            reward -= self.TARGET_COLLISION_PENALTY
        
        if self.end_effector_collision:
            reward -= self.END_EFFECTOR_COLLISION_PENALTY
        
        if self.forbidden_area_collision:
            reward -= self.END_EFFECTOR_COLLISION_PENALTY
        
        if self.elbow_target_collision:
            reward -= self.END_EFFECTOR_COLLISION_PENALTY
        
        # If we've fallen off the table, penalize this behaviour
        if self.chaser_position[0] > 4 or self.chaser_position[0] < -1 or self.chaser_position[1] > 3 or self.chaser_position[1] < -1 or self.chaser_position[2] > 4*np.pi or self.chaser_position[2] < -4*np.pi:
            reward -= self.FALL_OFF_TABLE_PENALTY

        return reward
    
    def check_collisions(self):
        """ Calculate whether the different objects are colliding with the target. 
        
            Updates 6 booleans: end_effector_collision, forbidden_area_collision, chaser_target_collision, docked, mid_way, elbow_target_collision
        """
        
        ##################################################
        ### Calculating Polygons in the inertial frame ###
        ##################################################
        
        # Target    
        target_points_body = np.array([[ self.LENGTH/2,-self.LENGTH/2],
                                       [-self.LENGTH/2,-self.LENGTH/2],
                                       [-self.LENGTH/2, self.LENGTH/2],
                                       [ self.LENGTH/2, self.LENGTH/2]]).T    
        # Rotation matrix (body -> inertial)
        C_Ib_target = self.make_C_bI(self.target_position[-1]).T        
        # Rotating body frame coordinates to inertial frame
        target_body_inertial = np.matmul(C_Ib_target, target_points_body) + np.array([self.target_position[0], self.target_position[1]]).reshape([2,-1])
        target_polygon = Polygon(target_body_inertial.T)
        
        # Forbidden Area
        forbidden_area_body = np.array([[self.LENGTH/2, self.LENGTH/2],   
                                        [self.DOCKING_PORT_CORNER1_POSITION[0],self.DOCKING_PORT_CORNER1_POSITION[1]],
                                        [self.DOCKING_PORT_MOUNT_POSITION[0],self.DOCKING_PORT_MOUNT_POSITION[1]],
                                        [self.DOCKING_PORT_CORNER2_POSITION[0],self.DOCKING_PORT_CORNER2_POSITION[1]],
                                        [-self.LENGTH/2,self.LENGTH/2],
                                        [self.LENGTH/2, self.LENGTH/2]]).T        
        # Rotating body frame coordinates to inertial frame
        forbidden_area_inertial = np.matmul(C_Ib_target, forbidden_area_body) + np.array([self.target_position[0], self.target_position[1]]).reshape([2,-1])         
        forbidden_polygon = Polygon(forbidden_area_inertial.T)
        
        # End-effector
        end_effector_point = Point(self.end_effector_position)
        
        # Chaser
        chaser_points_body = np.array([[ self.LENGTH/2,-self.LENGTH/2],
                                       [-self.LENGTH/2,-self.LENGTH/2],
                                       [-self.LENGTH/2, self.LENGTH/2],
                                       [ self.LENGTH/2, self.LENGTH/2]]).T    
        # Rotation matrix (body -> inertial)
        C_Ib_chaser = self.make_C_bI(self.chaser_position[-1]).T        
        # Rotating body frame coordinates to inertial frame
        chaser_body_inertial = np.matmul(C_Ib_chaser, chaser_points_body) + np.array([self.chaser_position[0], self.chaser_position[1]]).reshape([2,-1])
        chaser_polygon = Polygon(chaser_body_inertial.T)
        
        # Docking Polygon (circle)
        docking_circle = Point(self.target_position[:-1] + np.matmul(C_Ib_target, self.DOCKING_PORT_MOUNT_POSITION)).buffer(self.SUCCESSFUL_DOCKING_RADIUS)
        
        # Mid-way Polygon (circle)
        mid_way_circle = Point(self.target_position[:-1] + np.matmul(C_Ib_target, self.DOCKING_PORT_MOUNT_POSITION)).buffer(self.MID_WAY_REWARD_RADIUS)
        
        # Elbow position in the inertial frame
        elbow_position = self.chaser_position[:-1] + np.matmul(C_Ib_chaser, self.ELBOW_POSITION)
        elbow_point = Point(elbow_position)
        
        
        ###########################
        ### Checking collisions ###
        ###########################
        self.end_effector_collision = False
        self.forbidden_area_collision = False
        self.chaser_target_collision = False
        self.docked = False
        self.mid_way = False
        self.elbow_target_collision = False
        
        if self.CHECK_END_EFFECTOR_COLLISION and end_effector_point.within(target_polygon):
            if self.test_time:
                print("End-effector colliding with the target!")
            self.end_effector_collision = True
        
        if self.CHECK_END_EFFECTOR_FORBIDDEN and end_effector_point.within(forbidden_polygon):
            if self.test_time:
                print("End-effector within the forbidden area!")
            self.forbidden_area_collision = True
        
        if self.CHECK_CHASER_TARGET_COLLISION and chaser_polygon.intersects(target_polygon):
            if self.test_time:
                print("Chaser/target collision")
            self.chaser_target_collision = True
        
        if self.GIVE_MID_WAY_REWARD and self.not_yet_mid_way and end_effector_point.within(mid_way_circle):
            if self.test_time:
                print("Mid Way!")
            self.mid_way = True
        
        if end_effector_point.within(docking_circle):
            if self.test_time:
                print("Docked!")
            self.docked = True
        
        # Elbow can enter the forbidden area
        if self.CHECK_END_EFFECTOR_COLLISION and elbow_point.within(target_polygon):
            if self.test_time:
                print("Elbow/target collision!")
            self.elbow_target_collision = True


    def is_done(self):
        # Checks if this episode is done or not
        """
            NOTE: THE ENVIRONMENT MUST RETURN done = True IF THE EPISODE HAS
                  REACHED ITS LAST TIMESTEP
        """

        # If we've docked with the target
        if self.docked:
            return True

        # If we've fallen off the table, end the episode
        if self.chaser_position[0] > 4 or self.chaser_position[0] < -1 or self.chaser_position[1] > 3 or self.chaser_position[1] < -1 or self.chaser_position[2] > 4*np.pi or self.chaser_position[2] < -4*np.pi:
            if self.test_time:
                print("Fell off table!")
            return True
        
        # If we want to end the episode during a collision
        if self.END_ON_COLLISION and np.any([self.end_effector_collision, self.forbidden_area_collision, self.chaser_target_collision, self.elbow_target_collision]):
            if self.test_time:
                print("Ending episode due to a collision")
            return True

        # If we've run out of timesteps
        if round(self.time/self.TIMESTEP) == self.MAX_NUMBER_OF_TIMESTEPS:
            return True
        
        # Continue the episode
        return False


    def generate_queue(self):
        # Generate the queues responsible for communicating with the agent
        self.agent_to_env = multiprocessing.Queue(maxsize = 1)
        self.env_to_agent = multiprocessing.Queue(maxsize = 1)

        return self.agent_to_env, self.env_to_agent

    
    def make_C_bI(self, angle):
        
        C_bI = np.array([[ np.cos(angle), np.sin(angle)],
                         [-np.sin(angle), np.cos(angle)]]) # [2, 2]        
        return C_bI
    
    
    def relative_pose_body_frame(self):
        # Calculate the relative_x, relative_y, relative_vx, relative_vy, relative_angle, relative_angular_velocity
        # All in the body frame
                
        chaser_angle = self.chaser_position[-1]        
        # Rotation matrix (inertial -> body)
        C_bI = self.make_C_bI(chaser_angle)
                
        # [X,Y] relative position in inertial frame
        relative_position_inertial = self.target_position[:-1] - self.chaser_position[:-1]    
        relative_position_body = np.matmul(C_bI, relative_position_inertial)
        
        # [X, Y] Relative velocity in inertial frame
        relative_velocity_inertial = self.target_velocity[:-1] - self.chaser_velocity[:-1]
        relative_velocity_body = np.matmul(C_bI, relative_velocity_inertial)
        
        # Relative angle and wrap it to [0, 2*np.pi]
        relative_angle = np.array([(self.target_position[-1] - self.chaser_position[-1])%(2*np.pi)])
        
        # Relative angular velocity
        relative_angular_velocity = np.array([self.target_velocity[-1] - self.chaser_velocity[-1]])

        return np.concatenate([relative_position_body, relative_velocity_body, relative_angle, relative_angular_velocity])


    def run(self):
        ###################################
        ##### Running the environment #####
        ###################################
        """
        This method is called when the environment process is launched by main.py.
        It is responsible for continually listening for an input action from the
        agent through a Queue. If an action is received, it is to step the environment
        and return the results.
        
        TOTAL_STATE_SIZE = 18 [relative_x, relative_y, relative_vx, relative_vy, relative_angle, relative_angular_velocity, 
                               chaser_x, chaser_y, chaser_theta, target_x, target_y, target_theta, 
                               chaser_vx, chaser_vy, chaser_omega, target_vx, target_vy, target_omega] *# Relative pose expressed in the chaser's body frame; everythign else in Inertial frame #*        
        """
        # Instructing this process to treat Ctrl+C events (called SIGINT) by going SIG_IGN (ignore).
        # This permits the process to continue upon a Ctrl+C event to allow for graceful quitting.
        signal.signal(signal.SIGINT, signal.SIG_IGN)
        
        # Loop until the process is terminated
        while True:
            # Blocks until the agent passes us an action
            action, *test_time = self.agent_to_env.get()

            if type(action) == bool:
                # The signal to reset the environment was received
                self.reset(action, test_time[0])
                
                # Return the TOTAL_STATE
                self.env_to_agent.put(np.concatenate([self.relative_pose_body_frame(), self.chaser_position, self.target_position, self.chaser_velocity, self.target_velocity]))

            else:
                
                # Delay the action by DYNAMICS_DELAY timesteps. The environment accumulates the action delay--the agent still thinks the sent action was used.
                if self.DYNAMICS_DELAY > 0:
                    self.action_delay_queue.put(action,False) # puts the current action to the bottom of the stack
                    action = self.action_delay_queue.get(False) # grabs the delayed action and treats it as truth.   
                
                # Rotating the action from the body frame into the inertial frame
                action[:-1] = np.matmul(self.make_C_bI(self.chaser_position[-1]).T, action[:-1])

                ################################
                ##### Step the environment #####
                ################################                
                reward, done = self.step(action)

                # Return (TOTAL_STATE, reward, done)
                self.env_to_agent.put((np.concatenate([self.relative_pose_body_frame(), self.chaser_position, self.target_position, self.chaser_velocity, self.target_velocity]), reward, done))


###################################################################
##### Generating kinematics equations representing the motion #####
###################################################################
def kinematics_equations_of_motion(state, t, parameters):
    # From the state, it returns the first derivative of the state
    
    # Unpacking the action from the parameters
    action = parameters[0]
    position_length = parameters[1]
    
    # state is [chaser position, chaser velocity]
    # its derivative is [velocity, acceleration]
    #position = state[:position_length] # [x, y, theta]
    velocity = state[position_length:] # [x_dot, y_dot, theta_dot]
    
    acceleration = action # [x_dot_dot, y_dot_dot, theta_dot_dot]

    # Building the derivative matrix.
    derivatives = np.concatenate([velocity, acceleration])

    return derivatives


#####################################################################
##### Generating the dynamics equations representing the motion #####
#####################################################################
def dynamics_equations_of_motion(state, t, parameters):
    # state = [chaser_x, chaser_y, chaser_z, chaser_theta, chaser_Vx, chaser_Vy, chaser_Vz]

    # Unpacking the state
    x, y, theta, xdot, ydot, thetadot = state
    control_effort, mass, inertia = parameters # unpacking parameters
    
    # Building the derivative matrix
    derivatives = np.array((xdot, ydot, thetadot, control_effort[0]/mass, control_effort[1]/mass, control_effort[2]/inertia)).squeeze()
  
    return derivatives


##########################################
##### Function to animate the motion #####
##########################################
def render(states, actions, instantaneous_reward_log, cumulative_reward_log, critic_distributions, target_critic_distributions, projected_target_distribution, bins, loss_log, episode_number, filename, save_directory):
    """
    TOTAL_STATE = [relative_x, relative_y, relative_vx, relative_vy, relative_angle, relative_angular_velocity, chaser_x, chaser_y, chaser_theta, target_x, target_y, target_theta, chaser_vx, chaser_vy, chaser_omega, target_vx, target_vy, target_omega] *# Relative pose expressed in the chaser's body frame; everythign else in Inertial frame #*
     """   
    
    # Load in a temporary environment, used to grab the physical parameters
    temp_env = Environment()

    # Checking if we want the additional reward and value distribution information
    extra_information = temp_env.ADDITIONAL_VALUE_INFO

    # Unpacking state
    chaser_x, chaser_y, chaser_theta = states[:,6], states[:,7], states[:,8]
    target_x, target_y, target_theta = states[:,9], states[:,10], states[:,11]

    # Extracting physical properties
    LENGTH = temp_env.LENGTH
    DOCKING_PORT_MOUNT_POSITION = temp_env.DOCKING_PORT_MOUNT_POSITION
    DOCKING_PORT_CORNER1_POSITION = temp_env.DOCKING_PORT_CORNER1_POSITION
    DOCKING_PORT_CORNER2_POSITION = temp_env.DOCKING_PORT_CORNER2_POSITION
    ARM_MOUNT_POSITION = temp_env.ARM_MOUNT_POSITION
    SHOULDER_POSITION = temp_env.SHOULDER_POSITION
    ELBOW_POSITION = temp_env.ELBOW_POSITION
    WRIST_POSITION = temp_env.WRIST_POSITION
    END_EFFECTOR_POSITION = temp_env.END_EFFECTOR_POSITION

    ########################################################
    # Calculating spacecraft corner locations through time #
    ########################################################
    
    # All the points to draw of the chaser (except the front-face)    
    chaser_points_body = np.array([[ LENGTH/2,-LENGTH/2],
                                   [-LENGTH/2,-LENGTH/2],
                                   [-LENGTH/2, LENGTH/2],
                                   [ LENGTH/2, LENGTH/2],
                                   [ARM_MOUNT_POSITION[0],ARM_MOUNT_POSITION[1]],
                                   [SHOULDER_POSITION[0],SHOULDER_POSITION[1]],
                                   [ELBOW_POSITION[0],ELBOW_POSITION[1]],
                                   [WRIST_POSITION[0],WRIST_POSITION[1]],
                                   [END_EFFECTOR_POSITION[0],END_EFFECTOR_POSITION[1]]]).T
    
    # The front-face points on the target
    chaser_front_face_body = np.array([[[ LENGTH/2],[ LENGTH/2]],
                                       [[ LENGTH/2],[-LENGTH/2]]]).squeeze().T

    # Rotation matrix (body -> inertial)
    C_Ib_chaser = np.moveaxis(np.array([[np.cos(chaser_theta), -np.sin(chaser_theta)],
                                        [np.sin(chaser_theta),  np.cos(chaser_theta)]]), source = 2, destination = 0) # [NUM_TIMESTEPS, 2, 2]
    
    # Rotating body frame coordinates to inertial frame    
    chaser_body_inertial       = np.matmul(C_Ib_chaser, chaser_points_body)     + np.array([chaser_x, chaser_y]).T.reshape([-1,2,1])
    chaser_front_face_inertial = np.matmul(C_Ib_chaser, chaser_front_face_body) + np.array([chaser_x, chaser_y]).T.reshape([-1,2,1])


      
    # All the points to draw of the target (except the front-face)     
    target_points_body = np.array([[ LENGTH/2,-LENGTH/2],
                                   [-LENGTH/2,-LENGTH/2],
                                   [-LENGTH/2, LENGTH/2],
                                   [ LENGTH/2, LENGTH/2],
                                   [DOCKING_PORT_MOUNT_POSITION[0], LENGTH/2], # artificially adding this to make the docking cone look better 
                                   [DOCKING_PORT_MOUNT_POSITION[0],DOCKING_PORT_MOUNT_POSITION[1]],
                                   [DOCKING_PORT_CORNER1_POSITION[0],DOCKING_PORT_CORNER1_POSITION[1]],
                                   [DOCKING_PORT_CORNER2_POSITION[0],DOCKING_PORT_CORNER2_POSITION[1]],
                                   [DOCKING_PORT_MOUNT_POSITION[0],DOCKING_PORT_MOUNT_POSITION[1]]]).T
    
    # The front-face points on the target
    target_front_face_body = np.array([[[ LENGTH/2],[ LENGTH/2]],
                                       [[ LENGTH/2],[-LENGTH/2]]]).squeeze().T

    # Rotation matrix (body -> inertial)
    C_Ib_target = np.moveaxis(np.array([[np.cos(target_theta), -np.sin(target_theta)],
                                        [np.sin(target_theta),  np.cos(target_theta)]]), source = 2, destination = 0) # [NUM_TIMESTEPS, 2, 2]
    
    # Rotating body frame coordinates to inertial frame
    target_body_inertial       = np.matmul(C_Ib_target, target_points_body)     + np.array([target_x, target_y]).T.reshape([-1,2,1])
    target_front_face_inertial = np.matmul(C_Ib_target, target_front_face_body) + np.array([target_x, target_y]).T.reshape([-1,2,1])

    #######################
    # Plotting the motion #
    #######################
    
    # Generating figure window
    figure = plt.figure(constrained_layout = True)
    figure.set_size_inches(5, 4, True)

    if extra_information:
        grid_spec = gridspec.GridSpec(nrows = 2, ncols = 3, figure = figure)
        subfig1 = figure.add_subplot(grid_spec[0,0], aspect = 'equal', autoscale_on = False, xlim = (0, 3.5), ylim = (0, 2.4))
        #subfig1 = figure.add_subplot(grid_spec[0,0], projection = '3d', aspect = 'equal', autoscale_on = False, xlim3d = (-5, 5), ylim3d = (-5, 5), zlim3d = (0, 10), xlabel = 'X (m)', ylabel = 'Y (m)', zlabel = 'Z (m)')
        subfig2 = figure.add_subplot(grid_spec[0,1], xlim = (np.min([np.min(instantaneous_reward_log), 0]) - (np.max(instantaneous_reward_log) - np.min(instantaneous_reward_log))*0.02, np.max([np.max(instantaneous_reward_log), 0]) + (np.max(instantaneous_reward_log) - np.min(instantaneous_reward_log))*0.02), ylim = (-0.5, 0.5))
        subfig3 = figure.add_subplot(grid_spec[0,2], xlim = (np.min(loss_log)-0.01, np.max(loss_log)+0.01), ylim = (-0.5, 0.5))
        subfig4 = figure.add_subplot(grid_spec[1,0], ylim = (0, 1.02))
        subfig5 = figure.add_subplot(grid_spec[1,1], ylim = (0, 1.02))
        subfig6 = figure.add_subplot(grid_spec[1,2], ylim = (0, 1.02))

        # Setting titles
        subfig1.set_xlabel("X (m)",    fontdict = {'fontsize': 8})
        subfig1.set_ylabel("Y (m)",    fontdict = {'fontsize': 8})
        subfig2.set_title("Timestep Reward",    fontdict = {'fontsize': 8})
        subfig3.set_title("Current loss",       fontdict = {'fontsize': 8})
        subfig4.set_title("Q-dist",             fontdict = {'fontsize': 8})
        subfig5.set_title("Target Q-dist",      fontdict = {'fontsize': 8})
        subfig6.set_title("Bellman projection", fontdict = {'fontsize': 8})

        # Changing around the axes
        subfig1.tick_params(labelsize = 8)
        subfig2.tick_params(which = 'both', left = False, labelleft = False, labelsize = 8)
        subfig3.tick_params(which = 'both', left = False, labelleft = False, labelsize = 8)
        subfig4.tick_params(which = 'both', left = False, labelleft = False, right = True, labelright = False, labelsize = 8)
        subfig5.tick_params(which = 'both', left = False, labelleft = False, right = True, labelright = False, labelsize = 8)
        subfig6.tick_params(which = 'both', left = False, labelleft = False, right = True, labelright = True, labelsize = 8)

        # Adding the grid
        subfig4.grid(True)
        subfig5.grid(True)
        subfig6.grid(True)

        # Setting appropriate axes ticks
        subfig2.set_xticks([np.min(instantaneous_reward_log), 0, np.max(instantaneous_reward_log)] if np.sign(np.min(instantaneous_reward_log)) != np.sign(np.max(instantaneous_reward_log)) else [np.min(instantaneous_reward_log), np.max(instantaneous_reward_log)])
        subfig3.set_xticks([np.min(loss_log), np.max(loss_log)])
        subfig4.set_xticks([bins[i*5] for i in range(round(len(bins)/5) + 1)])
        subfig4.tick_params(axis = 'x', labelrotation = -90)
        subfig4.set_yticks([0, 0.2, 0.4, 0.6, 0.8, 1.])
        subfig5.set_xticks([bins[i*5] for i in range(round(len(bins)/5) + 1)])
        subfig5.tick_params(axis = 'x', labelrotation = -90)
        subfig5.set_yticks([0, 0.2, 0.4, 0.6, 0.8, 1.])
        subfig6.set_xticks([bins[i*5] for i in range(round(len(bins)/5) + 1)])
        subfig6.tick_params(axis = 'x', labelrotation = -90)
        subfig6.set_yticks([0, 0.2, 0.4, 0.6, 0.8, 1.])

    else:
        subfig1 = figure.add_subplot(1, 1, 1, aspect = 'equal', autoscale_on = False, xlim = (0, 3.5), ylim = (0, 2.4), xlabel = 'X Position (m)', ylabel = 'Y Position (m)')
     

    # Defining plotting objects that change each frame
    chaser_body,       = subfig1.plot([], [], color = 'r', linestyle = '-', linewidth = 2) # Note, the comma is needed
    chaser_front_face, = subfig1.plot([], [], color = 'k', linestyle = '-', linewidth = 2) # Note, the comma is needed
    target_body,       = subfig1.plot([], [], color = 'g', linestyle = '-', linewidth = 2)
    target_front_face, = subfig1.plot([], [], color = 'k', linestyle = '-', linewidth = 2)
    chaser_body_dot    = subfig1.scatter(0., 0., color = 'r', s = 0.1)

    if extra_information:
        reward_bar           = subfig2.barh(y = 0, height = 0.2, width = 0)
        loss_bar             = subfig3.barh(y = 0, height = 0.2, width = 0)
        q_dist_bar           = subfig4.bar(x = bins, height = np.zeros(shape = len(bins)), width = bins[1]-bins[0])
        target_q_dist_bar    = subfig5.bar(x = bins, height = np.zeros(shape = len(bins)), width = bins[1]-bins[0])
        projected_q_dist_bar = subfig6.bar(x = bins, height = np.zeros(shape = len(bins)), width = bins[1]-bins[0])
        time_text            = subfig1.text(x = 0.2, y = 0.91, s = '', fontsize = 8, transform=subfig1.transAxes)
        reward_text          = subfig1.text(x = 0.0, y = 1.02, s = '', fontsize = 8, transform=subfig1.transAxes)
    else:        
        time_text    = subfig1.text(x = 0.1, y = 0.9, s = '', fontsize = 8, transform=subfig1.transAxes)
        reward_text  = subfig1.text(x = 0.62, y = 0.9, s = '', fontsize = 8, transform=subfig1.transAxes)
        episode_text = subfig1.text(x = 0.4, y = 0.96, s = '', fontsize = 8, transform=subfig1.transAxes)
        episode_text.set_text('Episode ' + str(episode_number))

    # Function called repeatedly to draw each frame
    def render_one_frame(frame, *fargs):
        temp_env = fargs[0] # Extract environment from passed args

        # Draw the chaser body
        chaser_body.set_data(chaser_body_inertial[frame,0,:], chaser_body_inertial[frame,1,:])

        # Draw the front face of the chaser body in a different colour
        chaser_front_face.set_data(chaser_front_face_inertial[frame,0,:], chaser_front_face_inertial[frame,1,:])

        # Draw the target body
        target_body.set_data(target_body_inertial[frame,0,:], target_body_inertial[frame,1,:])

        # Draw the front face of the target body in a different colour
        target_front_face.set_data(target_front_face_inertial[frame,0,:], target_front_face_inertial[frame,1,:])

        # Drawing a dot in the centre of the chaser
        chaser_body_dot.set_offsets(np.hstack((chaser_x[frame],chaser_y[frame])))

        # Update the time text
        time_text.set_text('Time = %.1f s' %(frame*temp_env.TIMESTEP))

        # Update the reward text
        reward_text.set_text('Total reward = %.1f' %cumulative_reward_log[frame])
        
        try:
            if extra_information:
                # Updating the instantaneous reward bar graph
                reward_bar[0].set_width(instantaneous_reward_log[frame])
                # And colouring it appropriately
                if instantaneous_reward_log[frame] < 0:
                    reward_bar[0].set_color('r')
                else:
                    reward_bar[0].set_color('g')
    
                # Updating the loss bar graph
                loss_bar[0].set_width(loss_log[frame])
    
                # Updating the q-distribution plot
                for this_bar, new_value in zip(q_dist_bar, critic_distributions[frame,:]):
                    this_bar.set_height(new_value)
    
                # Updating the target q-distribution plot
                for this_bar, new_value in zip(target_q_dist_bar, target_critic_distributions[frame, :]):
                    this_bar.set_height(new_value)
    
                # Updating the projected target q-distribution plot
                for this_bar, new_value in zip(projected_q_dist_bar, projected_target_distribution[frame, :]):
                    this_bar.set_height(new_value)
        except:
            pass
    #
        # Since blit = True, must return everything that has changed at this frame
        return chaser_body_dot, time_text, chaser_body, chaser_front_face, target_body, target_front_face 

    # Generate the animation!
    fargs = [temp_env] # bundling additional arguments
    animator = animation.FuncAnimation(figure, render_one_frame, frames = np.linspace(0, len(states)-1, len(states)).astype(int),
                                       blit = False, fargs = fargs)

    """
    frames = the int that is passed to render_one_frame. I use it to selectively plot certain data
    fargs = additional arguments for render_one_frame
    interval = delay between frames in ms
    """

    # Save the animation!
    if temp_env.SKIP_FAILED_ANIMATIONS:
        try:
            # Save it to the working directory [have to], then move it to the proper folder
            animator.save(filename = filename + '_episode_' + str(episode_number) + '.mp4', fps = 30, dpi = 100)
            # Make directory if it doesn't already exist
            os.makedirs(os.path.dirname(save_directory + filename + '/videos/'), exist_ok=True)
            # Move animation to the proper directory
            os.rename(filename + '_episode_' + str(episode_number) + '.mp4', save_directory + filename + '/videos/episode_' + str(episode_number) + '.mp4')
        except:
            ("Skipping animation for episode %i due to an error" %episode_number)
            # Try to delete the partially completed video file
            try:
                os.remove(filename + '_episode_' + str(episode_number) + '.mp4')
            except:
                pass
    else:
        # Save it to the working directory [have to], then move it to the proper folder
        animator.save(filename = filename + '_episode_' + str(episode_number) + '.mp4', fps = 30, dpi = 100)
        # Make directory if it doesn't already exist
        os.makedirs(os.path.dirname(save_directory + filename + '/videos/'), exist_ok=True)
        # Move animation to the proper directory
        os.rename(filename + '_episode_' + str(episode_number) + '.mp4', save_directory + filename + '/videos/episode_' + str(episode_number) + '.mp4')

    del temp_env
    plt.close(figure)