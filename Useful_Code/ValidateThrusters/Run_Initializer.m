clear;
clc;
close all force;

warning('off','all')

%% User-defined constants:

% Converting from degrees to radians and vis versa:

d2r                            = pi/180;
r2d                            = 180/pi;

% Initialize the table size for use in the GUI (don't delete):

xLength                        = 3.51155;   % [m]
yLength                        = 2.41935;   % [m]


%% Set the base sampling rate: 

% This variable will change the frequency at which the template runs. If
% the frequency of the template changes, the frequency of the server must
% also be changed, i.e. open the StreamData.sln under the PhaseSpace Server
% folder, and change line 204 from owl.frequency(10) to 
% owl.frequency(serverRate):

baseRate                       = 0.05;      % 20 Hz

%% Set the thruster ON times for testing:

Thruster1_OnTime               = 1;
Thruster2_OnTime               = 2;
Thruster3_OnTime               = 3;
Thruster4_OnTime               = 4;
Thruster5_OnTime               = 5;
Thruster6_OnTime               = 6;
Thruster7_OnTime               = 7;
Thruster8_OnTime               = 8;
ThrusterOff_Time               = 9;

tsim                           = Thruster1_OnTime+Thruster2_OnTime+...
                                 Thruster3_OnTime+Thruster4_OnTime+...
                                 Thruster5_OnTime+Thruster6_OnTime+...
                                 Thruster7_OnTime+Thruster8_OnTime;
