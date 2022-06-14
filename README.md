# Welcome to SPOT

This testbed is used by researchers to investigate robotics, control and computer vision technologies enabling spacecraft proximity operation tasks, such as inspection maneuvers, rendezvous and docking, robotic capture of a tumbling target, and on-orbit assembly of large structure. SPOT consists of two air-bearing spacecraft platforms operating in close proximity on a granite surface. The use of air bearing on the platforms reduces the friction to a negligible level. Both platforms are actuated by compressed air expelled through small air nozzles, to provide three degree-of-freedom (3DOF) control authority. One platform contains a reaction wheel. The motion of both platforms is measured in real-time through LEDs which are tracked by a motion capture system. This provides highly accurate ground truth position and attitude data to evaluate the performance of the new robotics, control, and computer vision technologies.

This repository contains the software developed for SPOT. The software is a mix of Simulink code (making use of existing code that allows us to deploy the Simulink diagrams to a Raspberry Pi), and custom device driver blocks written in C++ code. It's been designed to be user-friendly, but is a constant work in progress and can be overwhelming for new users. This repository also contains a detailed Wiki on the entire laboratory in an attempt to help guide new users. New users should read this Wiki carefully and watch any related videos before conducting an experiment. 

To get started, the following information may be useful:
  - "Home" is organized according to the different sections of the wiki. It is the table of contents. Look through the table of contents and familiarize yourself with it.
  - "Overview of the Spacecraft Proximity Operations Testbed" provides extensive details on the various hardware and software features. Read it!
  - "Compressor Instructions" provides instructions on... the compressor! You won't get very far without knowing how to fill the air tanks, so ensure you know how to do it properly and (most importantly) safely.
  - "Initial Software Setup" will tell you what to install on your personal computer to run the software directly from it. This is great for developing experiments at home, and it is even possible to run experiments from your own computer if you make it the ground station.
  - "Setting up the Laboratory for an Experiment" walks you through the steps to properly set up the lab for an experiment, including cleaning procedures.
  - "Running a Simulation/Experiment" provide a step-by-step guide to running a simulation and an experiment. Useful!
  - "Frequently Asked Questions" contains Q&A's that have come up many times in the past. Before submitting a bug or software request, check this list!

The Wiki contains even more useful information then what was listed above, and it's HIGHLY recommended that new users read through the entire Wiki. Seriously, **READ it first, ASK questions after**.
