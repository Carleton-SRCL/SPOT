jetsonRepeater.py is run on the Jetson and communicates to: 1) the Pi; 2) any other scripts running on the Jetson
use_deep_guidance_arm.py is an example of another script that is run on the Jetson and interfaces with the Pi via the jetsonRepeater.py
The remaining scripts are used to capture or perform CNN-inference with the Zed camera.