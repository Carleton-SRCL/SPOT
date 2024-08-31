# Welcome to SPOT

This testbed is used by researchers to investigate robotics, control and computer vision technologies enabling spacecraft proximity operation tasks, such as inspection manoeuvres, rendezvous and docking, robotic capture of a tumbling target, and on-orbit assembly of large structures. SPOT consists of three air-bearing spacecraft platforms operating in close proximity on a granite surface. The use of air bearings on the platforms reduces the friction to a negligible level. Each platform is actuated by compressed air expelled through small air nozzles, which provides three degree-of-freedom (3DOF) control authority. The motion of all platforms is measured in real-time through LEDs which are tracked by a motion capture system. This provides highly accurate ground truth position and attitude data to evaluate the performance of the new robotics, control, and computer vision technologies.

This repository contains the software developed for SPOT. The software is a mix of Simulink code (making use of existing code that allows us to deploy the Simulink diagram to onboard NVIDIA Jetson Xavier NX computers), and custom device driver blocks written in C++ code. It has been designed to be user-friendly, but is a constant work in progress so feedback and bug reports are appreciated. This repository also contains a detailed Wiki on the entire laboratory in an attempt to help guide new users. New users should read this Wiki carefully and watch any related videos before conducting an experiment. 
