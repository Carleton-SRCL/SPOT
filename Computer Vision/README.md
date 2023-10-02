Computer Vision ReadMe File.
Written by: 2Arms2Legs

**SETUP Procedure.**

**FOR MAC USERS:**
Must have OS version up to at least 10.15.7, otherwise cannot work on mac

Install python 3.8.10 from: https://www.python.org/downloads/release/python-380/
    - Select and download "macOS 64-bit installer"

Install pip:
- python3 -m pip install PACKAGE

Install Xcode:
- For old OS users, sign up as a developer in Apple and download the **proper** version/ sign up here: https://developer.apple.com/download/all/?q=xcode
      - For 10.15.7 this is 12.4
      - Otherwise, google your current OS and find the compatible version of Xcode.
- Active its licence by opening terminal from your mac applications and run the line: sudo xcodebuild -license

Install VSCode:
- https://code.visualstudio.com/docs/languages/python
- Set up your working folder in VScode to store your files and such.
- Within the terminal type "python3 -v" to check which version, this may not match up yet
- press command-shift-p and select "Python:select Interpreter", select python version 3.8
- Running python3 in the terminal now should use python 3.8.10

Ensure wheels are up to date:
- Within the terminal of VScode run: python3 -m pip install --upgrade pip setuptools wheel

Download Numpy:
- Within the terminal of VScode run: python3 -m pip install numpy

Download openCV:
- Within the terminal of VScode run:python3 -m pip install opencv-contrib-python
- this will take about 30 min to an hour ^

Check
- To ensure the package has installed correctly in a new python file within VScode type: import cv2
- Now typing on a new line type: "cv2.aruco. " , leaving the last period open, a large list of functions within the aruco package should appear to use.
- Otherwise try out one of the Cvis files to see if it recognizes the functions

**For Windows Users**
Delete all current versions of Python on your computer, otherwise VScode will get confused and try to run the latest version

Install python 3.8.10 from: https://www.python.org/downloads/release/python-380/
    - Select and download "Windows x86-64 executable installer"

Install pip:
- Check if its already installed: python3 -m pip --version
- Otherwise install: py -m ensurepip --default-pip

Install VSCode:
- https://code.visualstudio.com/docs/languages/python
- Set up your working folder in VScode to store your files and such.
- Within the terminal type "python3 -v" to check which version, this may not match up yet
- press command-shift-p and select "Python:select Interpreter", select python version 3.8
- Running python3 in the terminal now should use python 3.8.10

Install Numpy:
- pip install numpy

Install opencv:
- pip install opencv-contrib-python



