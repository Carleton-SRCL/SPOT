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

d2r = pi/180;
r2d = 180/pi;
step = 0.1; %simulation step size
control_step = 0.5;

m_red = 12;%11.297 %spacecraft mass (kg)

I_z_red = 0.19816; %spacecraft moment of inertia about z-axis (kg*m^2)

B =diag([1/m_red, 1/m_red, 1/I_z_red]);
Binv = inv(B);

% % Platform Parameters
% RED_ini = [0.8; 2; 315*d2r; 0; 0; 0.01]; % IC of RED
% BLACK_ini = [2.5; 0.75; 90*d2r; 0; 0; -1.5*d2r]; % IC of BLACK
% BLUE_ini = [1.5; 1; 8*d2r; 0; 0; 1.5*d2r]; % IC of BLUE

%%%%% Attitude Mode %%%%%
%Mode = 0: Constantly Look At Target
%Mode = 1: Use APF Attitude
%select the method of determining the desired attitude
Att_Mode = 0;

%%%%% Test Case %%%%%
%Test Case 1 = Used last semester during experiments 
%Test Case 2 = Used last semester for comparing 
%Test Case 3 = All platforms move, designed by Laurenne
%Test Case 4 = Extra spicy moving and rotating target, designed by Adrian
%Test Case 5 = Test Case 1 + 180deg
%select test case or design your own and add it to the if statment below
Test_Case = 5;

%freeze platforms after this amount of time so they dont go off table
end_time_blue = 300;
end_time_black = 300;

%initial conditions
if Test_Case == 1
    RED_ini = [0.8, 2, deg2rad(315), 0, 0, 0];
    BLACK_ini = [2.5, 0.75, deg2rad(90), 0, 0, deg2rad(-1.5)];
    BLUE_ini = [1.5, 1, deg2rad(8), 0, 0, deg2rad(1.5)];
    BLACK_end = [2.5, 0.75, deg2rad(-90)];
    BLUE_end = [1.5, 1, deg2rad(188)];
elseif Test_Case == 2 %%%%wrong test case 2 parameters need to fix%%%%%%%%
    RED_ini = [3, 2, deg2rad(300), 0, 0, 0];
    BLACK_ini = [1, 1.25, deg2rad(235), 0, 0, deg2rad(1.43)];
    BLUE_ini = [1.75, 1.75, 0, 1.25/120, -1.25/120, 0];
    BLACK_end = [1, 1.25, deg2rad(406.6)];
    BLUE_end = [3,0.5,0]; 
elseif Test_Case == 3
    RED_ini = [2.8, 0.5, 0, 0, 0, 0];
    BLACK_ini = [0.75, 1.5, 1, 0.006, -0.01, -0.02];
    BLUE_ini = [2.3, 0.9, 0, 0.003, 0.009, 0.05];
    BLACK_end = [1.45, 0.281, -1.40];
    BLUE_end = [2.65, 2, 6.00];
elseif Test_Case == 4
    RED_ini = [0.25, 1.5, 0, 0, 0, 0];
    BLACK_ini = [2.5, 0.75, -2, 0, 0.01, -0.013];
    BLUE_ini = [1, 2, 0, 0, -0.01, 0.01];
    BLACK_end = [2.49, 1.97, -3.56];
    BLUE_end = [1.01, 0.78, 1.20];
elseif Test_Case == 5 % Test_Case == 5
    RED_ini = [2.7, 0.4, deg2rad(-180), 0, 0, 0];
    BLUE_ini = [1.3, 1.25, deg2rad(105), 0, 0, deg2rad(-1.5)];
    BLACK_ini = [1.75, 0.5, deg2rad(188), 0, 0, deg2rad(1.5)];
    BLUE_end = [1.3, 1.25, deg2rad(105+180)];
    BLACK_end = [1.75, 0.5, deg2rad(368)];
else
    RED_ini = [2.7, 0.4, deg2rad(135+360), 0, 0, 0];
    BLUE_ini = [1.3, 1.25, deg2rad(270), 0, 0, deg2rad(-1.5)];
    BLACK_ini = [2, 1.4, deg2rad(188), 0, 0, deg2rad(1.5)];
    BLUE_end = [1.3, 1.25, deg2rad(90)];
    BLACK_end = [2, 1.4, deg2rad(368)];
end

%set initial states (so they dont have to be imported every time)
drop_states_RED = RED_ini(1:3);
home_states_RED = RED_ini(1:3);
init_states_RED = RED_ini(1:3);

drop_states_BLACK = BLACK_ini(1:3);
home_states_BLACK = BLACK_ini(1:3);
init_states_BLACK = BLACK_ini(1:3);

drop_states_BLUE = BLUE_ini(1:3);
home_states_BLUE = BLUE_ini(1:3);
init_states_BLUE = BLUE_ini(1:3);

rc = 0.15; %m, chaser (RED) radius to docking port

u_max = 0.15; %N

tau_max = 2*u_max*rc;
max_input = [u_max;u_max;tau_max];

distb = 0; %magnitude of disturbance vector w (1.5e-2 is good)

sd = distb./3;
variance = sd*sd;

% APF Parameters
a1 = 0.3; % m 0.4
a2 = a1; % m
a3 = 0.4; % m 0.3
b1 = 0.175; % m 0.17
b2 = 0.175; % m 0.17
d_off = 0.2262; % m
r_hold = 0.427; % m
r_off = 0.165; % m

Beta = 30*d2r; %Docking corridor half-angle
ell_c = 0.5; %Docking corridor length (m)
al1 = pi/2-Beta;
al2 = pi/2 + Beta;


shape = [a1, a2, a3, b1, b2]; %parameters for shape of target potential function

ka = 1; %Attractive Constant
kr = 2; %Repulsive Constant 0.8

Q_a = diag([5, 5, 0]); %Positive definite shaping matrix for attraction diag([0.4, 0.4, 0.5])
Q_b = diag([0.0125, 0.0125, 0.05]); %Positive definite shaping matrix for repulsion (No needed for modified APF)
P_b = diag([45 45 0]); %Positive definite shaping matrix for repulsion diag([42 42 0])
K_a = diag([2 2 3.5]);
Nmat = diag([1 1 1]);

%PD control gains
%x-direction
Kp_x = 3.5;
Kd_x = 28;% 23
%y-direction
Kp_y = Kp_x;
Kd_y = Kd_x;
%rotation angle
Kp_t = 1;
Kd_t = 9.5;%25


%Obstacle field shaping parameters NOTE: (For no obstacle set psi to 0)
psi = 1;%1
sigma = 0.1;

%%%%%%%%%%%%%%%%%% Gradient Descent Method Selection: %%%%%%%%%%%%%%%%%%%
% Batch = 1; Momentum = 2; Nesterov = 3; 
% AdaGrad = 4; AdaDelta = 5; RMSProp = 6; Adam = 7; AdaMax = 8   
% (Choose a number from 1-8 for a gradient descent method)
Method = 8;

%%%%% Grdient Descent Constants %%%%%
%----Batch
%learning rate
batch_eta = 0.25;

%----Momentum
%learning rate
momentum_eta = 0.7;
%momentum term(usually near 0.9)(needs to be <1 to reach terminal velocity)
momentum_gamma = 0.7;

%----Nesterov
%learning rate
nesterov_eta = 0.6;
%momenum term(usually near 0.9)(needs to be <1 to reach terminal velocity)
nesterov_gamma = 0.3;

%----Adagrad
%learning rate(usually 0.01)
adagrad_eta = 0.18;
%smoothing term(usually near 0 to prevent division by zero)
adagrad_epsilon = 1e-8;

%----Adadelta
%momenum term(usually near 0.9)(needs to be <1 to reach terminal velocity)
adadelta_gamma = 0.37;
%smoothing term(usually near 0 to prevent division by zero)
adadelta_epsilon = 1e-5;

%----RMSprop
%learning rate(usually 0.01)
RMSprop_eta = 0.025;
%momenum term(usually near 0.9)(needs to be <1 to reach terminal velocity)
RMSprop_gamma = 0.9;
%smoothing term(usually near 0 to prevent division by zero)
RMSprop_epsilon = 1e-8;

%----Adam
%learning rate(usually 0.01)
adam_eta = 0.0163;
%smoothing term(usually near 0 to prevent division by zero)
adam_epsilon = 1e-2;
%beta 1 running average weighting factor
adam_beta_1 = 0.99;
%beta 2 squared running average weighting factor
adam_beta_2 = 0.999;

%----AdaMax
%learning rate(usually 0.01)
adamax_eta = 0.014;
%beta 1 running average weighting factor
adamax_beta_1 = 0.5;
%beta 2 squared running average weighting factor
adamax_beta_2 = 0.999;

%----Nadam
%learning rate(usually 0.01)
nadam_eta = 0.021;
%smoothing term(usually near 0 to prevent division by zero)
nadam_epsilon = 1e-8;
%beta 1 running average weighting factor
nadam_beta_1 = 0.5;
%beta 2 squared running average weighting factor
nadam_beta_2 = 0.999;

% %Learning Rate
% eta = 0.6;
% 
% %Momentum Constant    
% gamma = 0.9;
% 
% %Smoothing Term
% epsilon = 1e-8;
% 
% %Decay Rates
% beta_1 = 0.1;
% beta_2 = 0.1;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Here are the control parameters for the manipulator.

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

Kp_sharm                       = 1.5;
Kd_sharm                       = 1.0;

Kp_elarm                       = 1.2;
Kd_elarm                       = 0.8;

Kp_wrarm                       = 2;
Kd_wrarm                       = 0.6;


% Define the model properties for the joint friction:
% Based on https://ieeexplore.ieee.org/document/1511048

%Shoulder
Gamma1_sh = 0.005; 
Gamma2_sh = 5;
Gamma3_sh = 40;
Gamma4_sh = 0.015; 
Gamma5_sh = 800; 
Gamma6_sh = 0.005;

%Elbow
Gamma1_el = 0.12; 
Gamma2_el = 5;
Gamma3_el = 10;
Gamma4_el = 0.039; 
Gamma5_el = 800;
Gamma6_el = 0.000001;

%Wrist
Gamma1_wr = 0.025;
Gamma2_wr = 5;
Gamma3_wr = 40;
Gamma4_wr = 0.029;
Gamma5_wr = 800; 
Gamma6_wr = 0.02;

% Set the PWM frequency
PWMFreq = 5; %[Hz]

CTL_NEW
