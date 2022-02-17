% The following script is the initializer for SPOT 3.0; in this script,
% users define all initials parameters and/or constants required for
% simulation and experiment.

% Version: 3.07 (Beta Release)

% Authors: Alexander Crain
% Legacy: David Rogers & Kirk Hovell

clear;
clc;
close all force;
addpath(genpath('../../Custom_Library'))

warning('off','all')

fprintf('|----------------------------------------------------------------|\n')
fprintf('|----------------------------------------------------------------|\n')
fprintf('|                       -------------------                      |\n')
fprintf('|                     | Welcome to SPOT 3.0 |                    |\n')
fprintf('|                       -------------------                      |\n')
fprintf('|                                                                |\n')
fprintf('|Authors (v3.0): Alex Crain                                      |\n')
fprintf('|Authors (v2.0): Alex Crain and Kirk Hovell                      |\n')
fprintf('|Authors (Legacy): Dave Rogers and Kirk Hovell                   |\n')
fprintf('|                                                                |\n')
fprintf('|Current Version: 3.07 (Beta Release)                            |\n')
fprintf('|                                                                |\n')
fprintf('|Last Edit: 2021-03-07                                           |\n')
fprintf('|                                                                |\n')
fprintf('|----------------------------------------------------------------|\n')
fprintf('|----------------------------------------------------------------|\n')

%% User-defined constants:
addpath('Basic Math');

% Converting from degrees to radians and vis versa:

d2r                            = pi/180;
r2d                            = 180/pi;

% Initialize the table size for use in the GUI (don't delete):

xLength                        = 3.51155;   % [m]
yLength                        = 2.41935;   % [m]

% Initialize the PID gains for the RED platform:

Kp_xr                          = 2;
Kd_xr                          = 5;

Kp_yr                          = 2;
Kd_yr                          = 5;

Kp_tr                          = 0.1;
Kd_tr                          = 0.4;

% Initialize the PID gains for the BLACK platform:

Kp_xb                          = 2;
Kd_xb                          = 5;

Kp_yb                          = 2;
Kd_yb                          = 5;

Kp_tb                          = 0.1;
Kd_tb                          = 0.4;

% Initialize the PID gains for the BLUE platform:

Kp_xblue                       = 2;
Kd_xblue                       = 5;

Kp_yblue                       = 2;
Kd_yblue                       = 5;

Kp_tblue                       = 0.1;
Kd_tblue                       = 0.4;

% Set the noise variance level for the RED and BLACK platforms:

noise_variance_RED             = 0;
noise_variance_BLACK           = 0;
noise_variance_BLUE            = 0;

%% CUSTOM CONTROLLER DESIGN:

% Chaser spacecraft control gain.
kd = 5.0; % P gain for the controller.

% Pick the home states and the maximum amount of time we expect
% an experiment to run:
Phase3_SubPhase4_Duration = 3.0*60;
home_states_RED = [xLength/2+0.4; yLength/2; 0]; % [m; m; rad]
home_states_BLACK = [xLength/2-0.4; yLength/2; 0]; % [m; m; rad]
    

%% Set the base sampling rate: 

% This variable will change the frequency at which the template runs. If
% the frequency of the template changes, the frequency of the server must
% also be changed, i.e. open the StreamData.sln under the PhaseSpace Server
% folder, and change line 204 from owl.frequency(10) to 
% owl.frequency(serverRate):

baseRate                       = 0.05;      % 20 Hz

%% Set the frequency that the data is being sent up from the PhaseSpace:

% This variable must be less then the baseRate; in simulation, setting this
% equal to the baseRate causes the simulation fail, while in experiment
% setting this equal to or higher then the baseRate causes the data to
% buffer in the UDP send.

serverRate                     = 0.1;       % 10 Hz

%% Set the duration of each major phase in the experiment, in seconds:

% Set the duration of the sub-phases. Sub-phases occur during the
% experiment phase (Phase3_Duration) and must be manually inserted into the
% diagram. The total duration of the sub-phases must equal the length of
% the Phase3_Duration.

Phase3_SubPhase1_Duration      = 5;        % [s]
Phase3_SubPhase2_Duration      = 5;        % [s]
Phase3_SubPhase3_Duration      = 1;        % [s]
% Phase3_SubPhase4_Duration      = 2*60;      % [s]
Phase3_SubPhase3_and_a_half    = 5; % Time for the chase to do nothing while the target spins up.


Phase0_Duration                = 10;        % [s]
Phase1_Duration                = 5;         % [s]
Phase2_Duration                = 20;        % [s]
Phase3_Duration                =   Phase3_SubPhase1_Duration... 
                                 + Phase3_SubPhase2_Duration...
                                 + Phase3_SubPhase3_Duration...
                                 + Phase3_SubPhase4_Duration + Phase3_SubPhase3_and_a_half;       % [s]
Phase4_Duration                = 20;        % [s]
Phase5_Duration                = 5;         % [s]

% Determine the total experiment time from the durations:

tsim                           = Phase0_Duration + Phase1_Duration + ...
                                 Phase2_Duration + Phase3_Duration + ...
                                 Phase4_Duration + Phase5_Duration;        

% Determine the start time of each phase based on the duration:

Phase0_End                     = Phase0_Duration;
Phase1_End                     = Phase0_Duration + Phase1_Duration;           
Phase2_End                     = Phase0_Duration + Phase1_Duration + ...
                                 Phase2_Duration;         
Phase3_End                     = Phase0_Duration + Phase1_Duration + ...
                                 Phase2_Duration + Phase3_Duration;      
Phase4_End                     = Phase0_Duration + Phase1_Duration + ...
                                 Phase2_Duration + Phase3_Duration + ...
                                 Phase4_Duration; 
Phase5_End                     = Phase0_Duration + Phase1_Duration + ...
                                 Phase2_Duration + Phase3_Duration + ...
                                 Phase4_Duration + Phase5_Duration;                              
                             
% Determine the start time of each sub-phase based on the duration:  

Phase3_SubPhase1_End           = Phase2_End + Phase3_SubPhase1_Duration;
Phase3_SubPhase2_End           = Phase2_End + Phase3_SubPhase1_Duration + ...
                                 Phase3_SubPhase2_Duration;
Phase3_SubPhase3_End_BLACK           = Phase2_End + Phase3_SubPhase1_Duration + ...
                                 Phase3_SubPhase2_Duration +...
                                 Phase3_SubPhase3_Duration + Phase3_SubPhase3_and_a_half;
                             

Phase3_SubPhase3_End_RED           = Phase2_End + Phase3_SubPhase1_Duration + ...
                                 Phase3_SubPhase2_Duration +...
                                 Phase3_SubPhase3_Duration ;                            

                             
Phase3_SubPhase4_End           = Phase2_End + Phase3_SubPhase1_Duration + ...
                                 Phase3_SubPhase2_Duration +...
                                 Phase3_SubPhase3_Duration +...
                                 Phase3_SubPhase4_Duration + Phase3_SubPhase3_and_a_half;                             
                          
%% Load in any required data:

% Define the mass properties for the RED, BLACK, and BLUE platforms:

model_param(1)                 = 16.9478; % RED Mass
model_param(2)                 = 0.2709;  % RED Inertia;
model_param(3)                 = 12.3341; % BLACK Mass
model_param(4)                 = 0.1880;  % BLACK Inertia
model_param(5)                 = 12.7621; % BLUE Mass
model_param(6)                 = 0.1930;  % BLUE Inertia

REDMass = model_param(1);
REDInertia = model_param(2);
BLACKMass = model_param(3);
BLACKInertia = model_param(4);

% Initialize the thruster positions for the RED, BLACK, and BLUE platforms,
% as well as the expected maximum forces. The expected forces will only 
% affect the simulations.

F_thrusters_RED               = 0.25.*ones(8,1);
F_thrusters_BLACK             = 0.25.*ones(8,1);
F_thrusters_BLUE              = 0.25.*ones(8,1);
thruster_dist2CG_RED          = [49.92;-78.08;70.46;-63.54;81.08;-50.42;57.44;-75.96];
thruster_dist2CG_BLACK        = [83.42;-52.58;55.94;-60.05;54.08;-53.92;77.06;-55.94];
thruster_dist2CG_BLUE         = [83.42;-52.58;55.94;-60.05;54.08;-53.92;77.06;-55.94];

%% SELECT WEIGHTS FOR PARAMETER OPTIMIZATION:
W_CLVF = [1,100];
W_LVF = [1,100];

%% TARGET SPACECRAFT PHYSICAL CHARACTERISTICS: 

% PARAMETERS I CANNOT CHANGE (physical characteristics of target).
theta_d             = 30*d2r;           % Angle of the docking cone.
d                   = [0.16;0.542;0];   % docking position.
d_norm              = sqrt(sum(d.^2));  % Norm of the docking position.
o_hat_prime         = [0;1;0];          % Orientation of the docking cone.

% For the max acceleration:
u_max_scalar = 0.4;                                 % Approx max force.
a_max               = u_max_scalar/BLACKMass;       % Max accel given max force.

%% SELECT INITIAL CONDITIONS:

% Setup all of our initial conditions:
ICStructure = getICStructure();

% Select the conditions:
initialConditionSet = 4;

w_body = ICStructure.w_body{initialConditionSet};
rT_I0 = ICStructure.rT_I0{initialConditionSet};
rC_I0 = ICStructure.rC_I0{initialConditionSet};

rC_T0 = rC_I0 - rT_I0;
vC_T0 = zeros(3,1);

% NOTE - according to my thesis work, the following rotational parameters
% apply to a spacecraft spinning at a constant rate:
%
% w_max == abs(w_body)
% rotNorm == w_body^2
% dockingPortNorm == w_body^2 * d_norm
w_max = abs(w_body);
rotNorm = w_body^2;
dockingPortNorm = rotNorm * d_norm;

%% SETUP THE SWITCH STRUCTURE:
% For the switching condition:
acceptableRadius        = 0.05;                     % a 5cm radius.
timeThreshold           = 5;                        % 5 seconds staying in the radius
cntThreshold            = timeThreshold/baseRate;
        
%% LVF DESIGN PROCEDURE

% Safe distance to stay away from the docking cone:
a_prime = 0.2; % Initial guess for a

% INITIALIZING LYAPUNOV GUESSES:
% An initial guess:
v_max = 0.01; 
fact = 95/100;
finalAngle = fact*pi;

muLimit = 1000;
muFact = 10;

tol = 10^-6;
mu = 0.1; % My weighting parameter for the perf vs. constraints.
gamma = 0.2; % My damping factor
beta = 0.1; % My reduction factor.

% The interior-point solver below is implemented in the
% LVF_parameter_optimization folder.

v_max = interiorPointLVF_heur(...
    a_prime, ...
    v_max, ...
    a_max, ...
    w_max, ...
    rotNorm, ...
    dockingPortNorm, ...
    theta_d, ...
    fact, ...
    W_LVF, ...
    mu, ...
    tol, ...
    gamma, ...
    beta, ...
    muFact, ...
    muLimit...
);

% What were our estimates?
T_est_LVF = T_heur_LVF(a_prime,finalAngle,v_max);
F_est_LVF = F_heur_LVF(a_prime,dockingPortNorm,finalAngle,v_max,rotNorm,w_max);

disp("LVF time estimate: " + num2str(T_est_LVF) + " seconds.")
disp("LVF deltaV estimate: " + num2str(F_est_LVF) + " m/s.")
                
%% CLVF DESIGN PROCEDURE
% PERFORMING THE DESIGN PROCEDURE OF THE THREE GAINS TO SELECT:

aTimesOVec = d + a_prime*o_hat_prime;
a = sqrt(sum(aTimesOVec.^2));
o_hat_B = aTimesOVec./a;

% INITIALIZE OUR PARAMETERS FOR THE SEARCH:
b = 1.0;
ka = 0.01;
kc = 0.01;

tol = 10^-5;
mu = 0.1; % My weighting parameter for the perf vs. constraints.
gamma = 0.2; % My damping factor
beta = 0.1; % My reduction factor.

muLimit = 1000;
muFact = 10.0;

% NOTE - according to my thesis work, the following rotational parameters
% apply to a spacecraft spinning at a constant rate:
%
% w_max == abs(w_body)
% rotNorm == w_body^2
% dockingPortNorm == w_body^2 * d_norm

% The interior-point solver below is implemented in the
% CLVF_parameter_optimization folder.

[b, kc, ka] = interiorPointCLVF_heur(...
    b, ...
    kc, ...
    ka, ...
    a, ...
    a_max,... 
    w_max, ...
    rotNorm, ...
    rC_T0, ...
    vC_T0, ...
    W_CLVF, ...
    mu, ...
    tol, ...
    gamma, ...
    beta, ...
    muFact, ...
    muLimit...
);

% What were our estimates?
T_est = T_heur(a,b,ka,kc,rC_T0);
F_est = F_heur(a,b,ka,kc,rC_T0,vC_T0,rotNorm,w_max);

disp("CLVF T estimate is: " + num2str(T_est) + " seconds.");
disp("CLVF Fuel estimate is: " + num2str(F_est) + " m/s.");

disp("new ka is " + num2str(ka));
disp("new kc is " + num2str(kc));
disp("new b is " + num2str(b));  

%% MPC DESIGN PROCEDURE:
% Select the weighting matrices:
R = eye(3);
Q = eye(6)*100;
Q_final = Q*100;

% Get the A_cone and b_cone [body frame] approximation of the docking cone:
C_CB = C3(pi/2);
[A_cone, b_cone] = return_square_cone(theta_d, d, C_CB);
MPCStructure = getMPCStructure(BLACKMass, b_cone, R, Q, Q_final, u_max_scalar, kc, aTimesOVec);

%%  Set the drop, initial, and home positions for each platform:

drop_states_RED           = rT_I0; % [m; m; rad]
drop_states_BLACK         = rC_I0;
drop_states_BLUE          = [ xLength/2+0.9; yLength/2+0.5; 0];         % [m; m; rad]

init_states_RED           = rT_I0;
init_states_BLACK         = rC_I0;
init_states_BLUE          = [ xLength/2+0.9; yLength/2+0.5; 0];      % [m; m; rad]

% home_states_RED           = [ xLength/2+0.7; yLength/2; pi]; % [m; m; rad]
% home_states_BLACK         = [ xLength/2; yLength/2; 0];  % [m; m; rad]
home_states_BLUE          = [ xLength/2-0.9; yLength/2+0.5; 0];  % [m; m; rad]
                                   
%% Start the graphical user interface:

run('GUI_v3_07')

