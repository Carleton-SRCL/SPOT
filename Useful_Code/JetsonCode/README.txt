jetsonRepeater.py is run on the Jetson and communicates to: 1) the Pi; 2) any other scripts running on the Jetson
use_deep_guidance_arm.py is an example of another script that is run on the Jetson and interfaces with the Pi via the jetsonRepeater.py
The remaining scripts are used to capture or perform CNN-inference with the Zed camera.

For a Simulink example of communicating with the Pi, see: https://github.com/Kirkados/SPOT/tree/main/Projects/Kirk_Phase3 and look at the Kirk_Phase3.slx diagram