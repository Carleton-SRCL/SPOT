% The following script is the initializer for SPOT 4.0; in this script,
% users define all initials parameters and/or constants required for
% simulation and experiment.

clear;
clc;
close all force;

warning('off','all')

%% Start the graphical user interface:

run('GUI_v4_0_Main');

%% Place any custom variables or overwriting variables in this section

% As an example, here are the control parameters for the manipulator.

% Set torque limits on joints

Tz_lim_sharm                   = .1; % Shoulder Joint [Nm]

Tz_lim_elarm                   = .1; % Elbow Joint [Nm]

Tz_lim_wrarm                   = .1; % Wrist Joint [Nm]

% Transpose Jacobian controller gains:

Kp = [0.08 0 0
      0    0.08 0
      0    0    0.002];
Kv = [0.05 0 0
      0    0.05 0
      0    0    0.005];

% Initialize the PID gains for the ARM:

Kp_sharm                       = 0.15;
Kd_sharm                       = 0.05;

Kp_elarm                       = 0.08;
Kd_elarm                       = 0.05;

Kp_wrarm                       = 0.007;
Kd_wrarm                       = 0.005;


% Define the model properties for the joint friction:
% Based on https://ieeexplore.ieee.org/document/1511048

Gamma1 = 0.0025;
Gamma2 = 100;
Gamma3 = 10;
Gamma4 = 0.001;
Gamma5 = 100;
Gamma6 = 0.001;