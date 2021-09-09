# Welcome to SPOT3

This testbed is used by researchers to investigate robotics, control and computer vision technologies enabling spacecraft proximity operation tasks, such as inspection maneuvers, rendezvous and docking, robotic capture of a tumbling target, and on-orbit assembly of large structure. SPOT consists of two air-bearing spacecraft platforms operating in close proximity on a 2.4 m x 3.7 m granite surface. The use of air bearing on the platforms reduces the friction to a negligible level. Both platforms are actuated by a reaction wheel and compressed air expelled through miniature air nozzles, to provide three degree-of-freedom (3DOF) control authority. The motion of both platforms is measured in real-time through LEDs which are tracked by an eight-camera motion capture system. This provides highly accurate ground truth position and attitude data to evaluate the performance of the new robotics, control and computer vision technologies.

# Bug Tracking

- 3.04a: Occasionally on first startup the pucks alternate between their on/off state. Hit "STOP" float; start the float again and the bug should be gone.The previous experiment diagram is still running but the user either started a new experiment, or tried to turn on the pucks.

- 3.04b: Very rarely, the PhaseSpace cameras will fail to get a lock on the LEDs. Hit the emergency stop and reset the experiment.

- 3.04c (FIXED AS OF 3.05): If a diagram has been run in external mode, all diagrams will switch from "build" to "build and run" with no warning to the user. The only way to get around this at the moment is to manually check that "build" is selected under the build actions.

- 3.05a: Animation may buffer and "Stop Animation" may not be instant.

- The Pi must be unplugged before plugging in the charging cable. Kirk believes this is because the platforms have a floating ground, which, upon being immediately being brought to the grid's ground, sends an electrical pulse through the platform which damages the Pi. A possible solution is to install a capacitor on the Negative line of the platform to absorb such pulses.
