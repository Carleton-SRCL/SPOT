"""
This Agent class generates one agent that will run episodes. The agent collects,
processes, and dumps data into the ReplayBuffer. It will occasionally update the
parameters used by its neural network by grabbing the most up-to-date ones from
the Learner.

The environment is not contained in this thread because it must be in its own
process. The agent communicates with the environment through two queues:
    agent_to_env - the agent passes actions or reset signals to the environment process
    env_to_agent - the environment returns results to the agent

@author: Kirk Hovell (khovell@gmail.com)
"""

import time
import tensorflow as tf
import numpy as np
import os
import queue
from collections import deque
from pyvirtualdisplay import Display # for rendering
import sys

from settings import Settings
from build_neural_networks import BuildActorNetwork
environment_file = __import__('environment_' + Settings.ENVIRONMENT) # importing the environment

# For printing out all variables and their sizes
def sizeof_fmt(num, suffix='B'):
    ''' by Fred Cirera,  https://stackoverflow.com/a/1094933/1870254, modified'''
    for unit in ['','Ki','Mi','Gi','Ti','Pi','Ei','Zi']:
        if abs(num) < 1024.0:
            return "%3.1f %s%s" % (num, unit, suffix)
        num /= 1024.0
    return "%.1f %s%s" % (num, 'Yi', suffix)


class Agent:

    def __init__(self, sess, n_agent, agent_to_env, env_to_agent, replay_buffer, writer, filename, learner_policy_parameters, agent_to_learner, learner_to_agent):

        print("Initializing agent " + str(n_agent) + "...")

        # Saving inputs to self object for later use
        self.n_agent = n_agent
        self.sess = sess
        self.replay_buffer = replay_buffer
        self.filename = filename
        self.learner_policy_parameters = learner_policy_parameters
        self.agent_to_env = agent_to_env
        self.env_to_agent = env_to_agent
        self.agent_to_learner = agent_to_learner
        self.learner_to_agent = learner_to_agent
        
        # Build this Agent's actor network
        self.build_actor()

        # Build the operations to update the actor network
        self.build_actor_update_operation()

        # Establish the summary functions for TensorBoard logging.
        self.create_summary_functions()
        self.writer = writer

        # If we want to record video, launch one hidden display
        if Settings.RECORD_VIDEO and self.n_agent == 1:
            self.display = Display(visible = False, size = (1400,900))
            self.display.start()

        print("Agent %i initialized!" % self.n_agent)


    
    def create_summary_functions(self):
        # Logging the timesteps used for each episode for each agent
        self.timestep_number_placeholder      = tf.placeholder(tf.float32)
        self.episode_reward_placeholder       = tf.placeholder(tf.float32)
        timestep_number_summary               = tf.summary.scalar("Agent_" + str(self.n_agent) + "/Number_of_timesteps", self.timestep_number_placeholder)
        episode_reward_summary                = tf.summary.scalar("Agent_" + str(self.n_agent) + "/Episode_reward", self.episode_reward_placeholder)
        self.regular_episode_summary          = tf.summary.merge([timestep_number_summary, episode_reward_summary])

        # If this is agent 1, the agent who will also test performance, additionally log the reward
        if self.n_agent == 1:
            test_time_episode_reward_summary  = tf.summary.scalar("Test_agent/Episode_reward", self.episode_reward_placeholder)
            test_time_timestep_number_summary = tf.summary.scalar("Test_agent/Number_of_timesteps", self.timestep_number_placeholder)
            self.test_time_episode_summary    = tf.summary.merge([test_time_episode_reward_summary, test_time_timestep_number_summary])


    def build_actor(self):
        # Generate the actor's policy neural network
        agent_name = 'agent_' + str(self.n_agent) # agent name 'agent_3', for example
        self.state_placeholder = tf.placeholder(dtype = tf.float32, shape = [None, Settings.OBSERVATION_SIZE], name = 'state_placeholder') # the * lets Settings.OBSERVATION_SIZE be not restricted to only a scalar

        #############################
        #### Generate this Actor ####
        #############################
        self.policy = BuildActorNetwork(self.state_placeholder, scope = agent_name)


    def build_actor_update_operation(self):
        # Update agent's policy network parameters from the most up-to-date version from the learner
        update_operations = []
        source_variables = self.learner_policy_parameters
        destination_variables = self.policy.parameters

        # For each parameters in the network
        for source_variable, destination_variable in zip(source_variables, destination_variables):
            # Directly copy from the learner to the agent
            update_operations.append(destination_variable.assign(source_variable))

        # Save the operation that performs the actor update
        self.update_actor_parameters = update_operations
    
    def reset_action_augment_log(self):
        # Create state-augmentation queue (holds previous actions)
        self.past_actions = queue.Queue(maxsize = Settings.AUGMENT_STATE_WITH_ACTION_LENGTH)
        
        # Fill it with zeros to start
        for i in range(Settings.AUGMENT_STATE_WITH_ACTION_LENGTH):
            self.past_actions.put(np.zeros(Settings.ACTION_SIZE), False)
            
    def augment_state_with_actions(self, total_state):
        # Just received a total_state from the environment, need to augment 
        # it with the past action data and return it
        
        past_action_data = np.asarray(self.past_actions.queue).reshape([-1]) # past actions reshaped into a column
        augmented_state = np.concatenate([total_state, past_action_data])
        
        # Remove the oldest entry from the action log queue
        self.past_actions.get(False)
        
        return augmented_state

    def run(self, stop_run_flag, replay_buffer_dump_flag, starting_episode_number):
        # Runs the agent in its own environment
        # Runs for a specified number of episodes or until told to stop
        print("Starting to run agent %i at episode %i." % (self.n_agent, starting_episode_number[self.n_agent -1]))

        # Initializing parameters for agent network
        self.sess.run(self.update_actor_parameters)

        # Getting the starting episode number. If we are restarting a training
        # run that has crashed, the starting episode number will not be 1.
        episode_number = starting_episode_number[self.n_agent - 1]

        # Resetting the noise scale
        noise_scale = 0.

        # Start time
        start_time = time.time()

        # Creating the temporary memory space for calculating N-step returns
        self.n_step_memory = deque()

        # For all requested episodes or until user flags for a stop (via Ctrl + C)
        while episode_number <= Settings.NUMBER_OF_EPISODES and not stop_run_flag.is_set():

            ####################################
            #### Getting this episode ready ####
            ####################################

            # Clearing the N-step memory for this episode
            self.n_step_memory.clear()
            
            # Reset the action_log, if applicable
            if Settings.AUGMENT_STATE_WITH_ACTION_LENGTH > 0:
                self.reset_action_augment_log()

            # Checking if this is a test time (when we run an agent in a
            # noise-free environment to see how the training is going).
            # Only agent_1 is used for test time
            test_time = (self.n_agent == 1) and (episode_number % Settings.CHECK_GREEDY_PERFORMANCE_EVERY_NUM_EPISODES == 0 or episode_number == 1)

            # Resetting the environment for this episode by sending a boolean
            if test_time and Settings.TEST_ON_DYNAMICS:
                self.agent_to_env.put((True, test_time)) # Reset into a dynamics environment only if it's test time and desired
            else:
                self.agent_to_env.put((False, test_time)) # Reset into a kinematics environment
            total_state = self.env_to_agent.get()
            
            # Augment total_state with past actions, if appropriate
            if Settings.AUGMENT_STATE_WITH_ACTION_LENGTH > 0:
                total_state = self.augment_state_with_actions(total_state)

            # Calculating the noise scale for this episode. The noise scale
            # allows for changing the amount of noise added to the actor during training.
            if test_time:
                # It's test time! Run this episode without noise (if desired) to evaluate performance.
                if Settings.NOISELESS_AT_TEST_TIME:
                    noise_scale = 0

                # Additionally, if it's time to render, make a statement to the user
                if Settings.RECORD_VIDEO and (episode_number % (Settings.CHECK_GREEDY_PERFORMANCE_EVERY_NUM_EPISODES*Settings.VIDEO_RECORD_FREQUENCY) == 0 or episode_number == 1):
                    # Also log the states & actions encountered in this episode because we are going to render them!
                    raw_total_state_log = []
                    observation_log = []
                    action_log = []
                    next_observation_log = []
                    instantaneous_reward_log = []
                    cumulative_reward_log = []
                    done_log = []
                    discount_factor_log = []
                    raw_total_state_log.append(total_state)
                    cumulative_reward_log.append(0) # initializing with 0 at the initial state
                    


            else:
                # Regular training episode, use noise.
                # Noise is decayed during the training
                noise_scale = Settings.NOISE_SCALE * Settings.NOISE_SCALE_DECAY ** episode_number

            # Normalizing the total_state to 1 separately along each dimension
            # to avoid the 'vanishing gradients' problem
            if Settings.NORMALIZE_STATE:
                total_state = (total_state - Settings.STATE_MEAN)/Settings.STATE_HALF_RANGE

            # Discarding irrelevant states to obtain the observation
            observation = np.delete(total_state, Settings.IRRELEVANT_STATES)

            # Resetting items for this episode
            episode_reward = 0
            timestep_number = 0
            done = False

            # Stepping through time until episode completes.
            while not done:
                ##############################
                ##### Running the Policy #####
                ##############################
                action = self.sess.run(self.policy.action_scaled, feed_dict = {self.state_placeholder: np.expand_dims(observation,0)})[0] # Expanding the observation to be a 1x3 instead of a 3

                # Calculating random action to be added to the noise chosen from the policy to force exploration.
                if Settings.UNIFORM_OR_GAUSSIAN_NOISE:
                    # Uniform noise (sampled between -/+ the action range)
                    exploration_noise = np.random.uniform(low = -Settings.ACTION_RANGE, high = Settings.ACTION_RANGE, size = Settings.ACTION_SIZE)*noise_scale
                else:
                    # Gaussian noise (standard normal distribution scaled to half the action range)
                    exploration_noise = np.random.normal(size = Settings.ACTION_SIZE)*Settings.ACTION_RANGE*noise_scale # random number multiplied by the action range

                # Add exploration noise to original action, and clip it incase we've exceeded the action bounds
                action = np.clip(action + exploration_noise, Settings.LOWER_ACTION_BOUND, Settings.UPPER_ACTION_BOUND)

                # Adding the action taken to the past_actions log
                if Settings.AUGMENT_STATE_WITH_ACTION_LENGTH > 0:
                    self.past_actions.put(action)

                ################################################
                #### Step the dynamics forward one timestep ####
                ################################################
                # Send the action to the environment process
                self.agent_to_env.put((action,))

                # Receive results from stepped environment
                next_total_state, reward, done = self.env_to_agent.get() # The * means the variable will be unpacked only if it exists

                # Add reward we just received to running total for this episode
                episode_reward += reward
                if self.n_agent == 1 and Settings.RECORD_VIDEO and (episode_number % (Settings.CHECK_GREEDY_PERFORMANCE_EVERY_NUM_EPISODES*Settings.VIDEO_RECORD_FREQUENCY) == 0 or episode_number == 1) and not Settings.ENVIRONMENT == 'gym':
                    cumulative_reward_log.append(episode_reward)
                
                # Augment total_state with past actions, if appropriate
                if Settings.AUGMENT_STATE_WITH_ACTION_LENGTH > 0:
                    next_total_state = self.augment_state_with_actions(next_total_state)

                if self.n_agent == 1 and Settings.RECORD_VIDEO and (episode_number % (Settings.CHECK_GREEDY_PERFORMANCE_EVERY_NUM_EPISODES*Settings.VIDEO_RECORD_FREQUENCY) == 0 or episode_number == 1) and not Settings.ENVIRONMENT == 'gym':
                    raw_total_state_log.append(next_total_state)

                # Normalize the state
                if Settings.NORMALIZE_STATE:
                    next_total_state = (next_total_state - Settings.STATE_MEAN)/Settings.STATE_HALF_RANGE

                # Discarding irrelevant states
                next_observation = np.delete(next_total_state, Settings.IRRELEVANT_STATES)

                # Store the data in this temporary buffer until we calculate the n-step return
                self.n_step_memory.append((observation, action, reward))

                # If the n-step memory is full enough and we aren't testing performance
                if (len(self.n_step_memory) >= Settings.N_STEP_RETURN):
                    # Grab the oldest data from the n-step memory
                    observation_0, action_0, reward_0 = self.n_step_memory.popleft()
                    # N-step reward starts with reward_0
                    n_step_reward = reward_0
                    # Initialize gamma
                    discount_factor = Settings.DISCOUNT_FACTOR
                    for (observation_i, action_i, reward_i) in self.n_step_memory:
                        # Calculate the n-step reward
                        n_step_reward += reward_i*discount_factor
                        discount_factor *= Settings.DISCOUNT_FACTOR # for the next step, gamma**(i+1)

                    # Dump data into large replay buffer
                    # If the prioritized replay buffer is currently dumping data,
                    # wait until that is done before adding more data to the buffer                    
                    if not test_time:
                        replay_buffer_dump_flag.wait() # blocks until replay_buffer_dump_flag is True
                        self.replay_buffer.add((observation_0, action_0, n_step_reward, next_observation, done, discount_factor))

                    # If this episode is being rendered, log the state for rendering later
                    if self.n_agent == 1 and Settings.RECORD_VIDEO and (episode_number % (Settings.CHECK_GREEDY_PERFORMANCE_EVERY_NUM_EPISODES*Settings.VIDEO_RECORD_FREQUENCY) == 0 or episode_number == 1) and not Settings.ENVIRONMENT == 'gym':
                        observation_log.append(observation_0)
                        action_log.append(action_0)
                        next_observation_log.append(next_observation)
                        instantaneous_reward_log.append(n_step_reward)
                        done_log.append(done)
                        discount_factor_log.append(discount_factor)

                # End of timestep -> next state becomes current state
                observation = next_observation
                timestep_number += 1

                # If this episode is done, drain the N-step buffer, calculate
                # returns, and dump in replay buffer unless it is test time.
                if done:
                    # Episode has just finished, calculate the remaining N-step entries
                    while len(self.n_step_memory) > 0:
                        # Grab the oldest data from the n-step memory
                        observation_0, action_0, reward_0 = self.n_step_memory.popleft()
                        # N-step reward starts with reward_0
                        n_step_reward = reward_0
                        # Initialize gamma
                        discount_factor = Settings.DISCOUNT_FACTOR
                        for (observation_i, action_i, reward_i) in self.n_step_memory:
                            # Calculate the n-step reward
                            n_step_reward += reward_i*discount_factor
                            discount_factor *= Settings.DISCOUNT_FACTOR # for the next step, gamma**(i+1)

                        # dump data into large replay buffer
                        if not test_time:
                            replay_buffer_dump_flag.wait()
                            self.replay_buffer.add((observation_0, action_0, n_step_reward, next_observation, done, discount_factor))

                        # If this episode is being rendered, log the state for rendering later
                        if self.n_agent == 1 and Settings.RECORD_VIDEO and (episode_number % (Settings.CHECK_GREEDY_PERFORMANCE_EVERY_NUM_EPISODES*Settings.VIDEO_RECORD_FREQUENCY) == 0 or episode_number == 1) and not Settings.ENVIRONMENT == 'gym':
                            observation_log.append(observation_0)
                            action_log.append(action_0)
                            next_observation_log.append(next_observation)
                            cumulative_reward_log.append(episode_reward)
                            instantaneous_reward_log.append(n_step_reward)
                            done_log.append(done)
                            discount_factor_log.append(discount_factor)

            ################################
            ####### Episode Complete #######
            ################################
            # If this episode is being rendered, render it now.
            if self.n_agent == 1 and Settings.RECORD_VIDEO and (episode_number % (Settings.CHECK_GREEDY_PERFORMANCE_EVERY_NUM_EPISODES*Settings.VIDEO_RECORD_FREQUENCY) == 0 or episode_number == 1) and not Settings.ENVIRONMENT == 'gym':
                print("Rendering Actor %i at episode %i" % (self.n_agent, episode_number))

                os.makedirs(os.path.dirname(Settings.MODEL_SAVE_DIRECTORY + self.filename + '/trajectories/'), exist_ok=True)
                np.savetxt(Settings.MODEL_SAVE_DIRECTORY + self.filename + '/trajectories/' + str(episode_number) + '.txt',np.asarray(raw_total_state_log))

                # Ask the learner to tell us the value distributions of the state-action pairs encountered in this episode
                self.agent_to_learner.put((np.asarray(observation_log), np.asarray(action_log), np.asarray(next_observation_log), np.asarray(instantaneous_reward_log), np.asarray(done_log), np.asarray(discount_factor_log)))

                # Wait for the results
                try:
                    critic_distributions, target_critic_distributions, projected_target_distribution, loss_log = self.learner_to_agent.get(timeout = 3)

                    bins = np.linspace(Settings.MIN_V, Settings.MAX_V, Settings.NUMBER_OF_BINS)

                    # Render the episode
                    environment_file.render(np.asarray(raw_total_state_log), np.asarray(action_log), np.asarray(instantaneous_reward_log), np.asarray(cumulative_reward_log), critic_distributions, target_critic_distributions, projected_target_distribution, bins, np.asarray(loss_log), episode_number, self.filename, Settings.MODEL_SAVE_DIRECTORY)

                except queue.Empty:
                    print("Skipping this animation!")

            # Periodically update the agent with the learner's most recent version of the actor network parameters
            if episode_number % Settings.UPDATE_ACTORS_EVERY_NUM_EPISODES == 0:
                self.sess.run(self.update_actor_parameters)

            # Periodically print to screen how long it's taking to run these episodes
            if episode_number % Settings.DISPLAY_ACTOR_PERFORMANCE_EVERY_NUM_EPISODES == 0:
                print("Actor " + str(self.n_agent) + " ran " + str(Settings.DISPLAY_ACTOR_PERFORMANCE_EVERY_NUM_EPISODES) + " episodes in %.1f minutes, and is now at episode %i" % ((time.time() - start_time)/60, episode_number))
                start_time = time.time()

            ###################################################
            ######## Log training data to tensorboard #########
            ###################################################
            # Logging the number of timesteps and the episode reward.
            feed_dict = {self.episode_reward_placeholder:  episode_reward, self.timestep_number_placeholder: timestep_number}
            if test_time:
                summary = self.sess.run(self.test_time_episode_summary, feed_dict = feed_dict)
            else:
                summary = self.sess.run(self.regular_episode_summary,   feed_dict = feed_dict)
            self.writer.add_summary(summary, episode_number)

            # Increment the episode counter
            episode_number += 1

        #################################
        ##### All episodes complete #####
        #################################
        # If were recording video, stop the display
        if Settings.RECORD_VIDEO and self.n_agent == 1:
            self.display.stop()

        # Notify user of completion
        print("Actor %i finished after running %i episodes!" % (self.n_agent, episode_number - 1))