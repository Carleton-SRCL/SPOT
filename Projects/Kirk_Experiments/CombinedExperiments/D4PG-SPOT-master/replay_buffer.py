"""
Generates and manages the large experience replay buffer.

The experience replay buffer holds all the data that is dumped into it from the 
many agents who are running episodes of their own. The learner then trains off 
this heap of data continually and in its own thread.

@author: Kirk Hovell (khovell@gmail.com)
"""

import random
import pickle
import numpy as np

from collections import deque

from settings import Settings

class ReplayBuffer():
    # Generates and manages a non-prioritized replay buffer
    
    def __init__(self, filename):
        
        # Save filename
        self.filename = filename
               
        # Generate the buffer
        self.buffer = deque(maxlen = Settings.REPLAY_BUFFER_SIZE)
        
        # Try to load in the filled buffer
        if Settings.RESUME_TRAINING:
            try:
                print("Loading in the saved replay buffer samples...", end = "")
                self.load()
                print("Success!")
            except:
                print("\n\nCouldn't load in pickle! Starting an empty buffer")

    # Query how many entries are in the buffer
    def how_filled(self):
        return len(self.buffer)
    
    # Add new experience to the buffer
    def add(self, experience):
        self.buffer.append(experience)
        
    # Randomly sample data from the buffer
    def sample(self):
        # Decide how much data to sample
        # (maybe the buffer doesn't contain enough samples yet to fill a MINI_BATCH)
        batch_size = min(Settings.MINI_BATCH_SIZE, len(self.buffer)) 
        # Sample the data
        sampled_batch = np.asarray(random.sample(self.buffer, batch_size))

        # Unpack the training data
        states_batch           = np.stack(sampled_batch[:, 0])
        actions_batch          = np.stack(sampled_batch[:, 1])
        rewards_batch          = sampled_batch[:, 2]
        next_states_batch      = np.stack(sampled_batch[:, 3])
        dones_batch            = np.stack(sampled_batch[:,4])
        gammas_batch           = np.reshape(sampled_batch[:, 5], [-1, 1])

        return states_batch, actions_batch, rewards_batch, next_states_batch, dones_batch, gammas_batch
    
    def save(self):
        print("Saving replay buffer with %i samples" %self.how_filled())
        # Saves the replay buffer to file for a backup
        with open(Settings.MODEL_SAVE_DIRECTORY + self.filename + '/replay_buffer_dump', 'wb') as pickle_file:
            try:
                pickle.dump(self.buffer, pickle_file)
            except:
                print("Save failed since the buffer was written to during the save.")
    
    def load(self):
        # Loads the replay buffer from file to continue training
        with open(Settings.MODEL_SAVE_DIRECTORY + self.filename + '/replay_buffer_dump', 'rb') as pickle_file:
            self.buffer = pickle.load(pickle_file)