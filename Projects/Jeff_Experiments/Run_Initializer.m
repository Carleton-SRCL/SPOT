% The following script is the initializer for SPOT 3.0; in this script,
% users define all initials parameters and/or constants required for
% simulation and experiment.

% Version: 3.07 (Beta Release)

% Authors: Alexander Crain
% Legacy: David Rogers & Kirk Hovell

clear;
clc;
close all force;

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
%% SELECT YOUR INITIAL CONDITIONS SET:
    initialConditionSet = 1;
    possibleStarts = linspace(0,8/5*pi,5);
    startingRotation = 2;
    
    if initialConditionSet == 1
        % Spin rate of the spacecraft:
            w_body = 7.0 * d2r;
            
        % Initial position of the Target:
            rT_I0 = [yLength/2;yLength/2;possibleStarts(startingRotation)]; % [m ; m ; rads]
            
        % Initial position of the Chaser:
            rC_I0 = [0.9*xLength;yLength/2-0.5;pi/2]; % [m ; m ; rads]
            
        % Amount of time this will take:
            Phase3_SubPhase4_Duration      = 3.0*60;      % [s]
            
        % Home States
            home_states_RED           = [ xLength/2+0.4; yLength/2; 0]; % [m; m; rad]
            home_states_BLACK         = [ xLength/2-0.4; yLength/2; 0];      % [m; m; rad]

    elseif initialConditionSet == 2
        % Spin rate of the spacecraft:
            w_body = -8.5 * d2r;
            
        % Initial position of the Target:
            rT_I0 = [yLength/2;yLength/2;possibleStarts(startingRotation)]; % [m ; m ; rads]
            
            
        % Initial position of the Chaser:
            rC_I0 = [0.9*xLength;yLength/2+0.5;pi/2]; % [m ; m ; rads]
            
        % Amount of time this will take:
            Phase3_SubPhase4_Duration      = 3.0*60;      % [s]
            
        % Home States
            home_states_RED           = [ xLength/2+0.4; yLength/2; 0]; % [m; m; rad]
            home_states_BLACK         = [ xLength/2-0.4; yLength/2; 0];      % [m; m; rad]
            
    elseif initialConditionSet == 3
        % Spin rate of the spacecraft:
            w_body = 8.0 * d2r;
            
        % Initial position of the Target:
            rT_I0 = [yLength/2;yLength/2;possibleStarts(startingRotation)]; % [m ; m ; rads]
            
            
        % Initial position of the Chaser:
            rC_I0 = [0.9*xLength;yLength/2-0.5;pi/2]; % [m ; m ; rads]
            
        % Amount of time this will take:
            Phase3_SubPhase4_Duration      = 3.0*60;      % [s]
            
        % Home states:
            home_states_RED           = [ xLength/2-0.4; yLength/2; 0];      % [m; m; rad]
            home_states_BLACK         = [ xLength/2+0.4; yLength/2; 0];      % [m; m; rad]
            
            
    elseif initialConditionSet == 4
        % Spin rate of the spacecraft:
            w_body = -6.0 * d2r;
            
        % Initial position of the Target:
            rT_I0 = [yLength/2;yLength/2;possibleStarts(startingRotation)]; % [m ; m ; rads]
            
            
        % Initial position of the Chaser:
            rC_I0 = [0.9*xLength;yLength/2+0.5;pi/2]; % [m ; m ; rads]
            
        % Amount of time this will take:
            Phase3_SubPhase4_Duration      = 3.0*60;      % [s]
            
        % Home states:
            home_states_RED           = [ xLength/2+0.4; yLength/2; 0];      % [m; m; rad]
            home_states_BLACK         = [ xLength/2-0.4; yLength/2; 0];      % [m; m; rad]
            
    elseif initialConditionSet == 5
        % Spin rate of the spacecraft:
            w_body = 6.0 * d2r;
            
        % Initial position of the Target:
            rT_I0 = [yLength/2;yLength/2;possibleStarts(startingRotation)]; % [m ; m ; rads]
            
            
        % Initial position of the Chaser:
            rC_I0 = [0.9*xLength;yLength/2-0.5;pi/2]; % [m ; m ; rads]
            
        % Amount of time this will take:
            Phase3_SubPhase4_Duration      = 3.0*60;      % [s]
            
        % Home states:
            home_states_RED           = [ xLength/2-0.4; yLength/2; 0];      % [m; m; rad]
            home_states_BLACK         = [ xLength/2+0.4; yLength/2; 0];      % [m; m; rad] 
        
    end
    
    % Chaser spacecraft control gain.
        kd = 5.0; % P gain for the controller.
    

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

%% PARAMETERS AND INITIAL CONDITIONS  

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%% FOR THE KALMAN FILTER DESIGN %%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  

        A_cont = [zeros(3,3) eye(3,3);zeros(3,6)]; B_cont_RED = [zeros(3,3);eye(3,3)/REDMass]; C_cont = [eye(3,3), zeros(3,3)]; D_cont = zeros(3,3);
        B_cont_BLACK = [zeros(3,3);eye(3,3)/BLACKMass];
        
        A_rot_cont = [0 1;0 0]; B_rot_cont_RED = [0;1/REDInertia]; C_rot_cont = [1 0]; D_rot_cont = 0; B_rot_cont_BLACK = [0;1/BLACKInertia];
        
        sys_RED_CONT = ss(A_cont,B_cont_RED,C_cont,D_cont);
        sys_BLACK_CONT = ss(A_cont,B_cont_BLACK,C_cont,D_cont);
        
        sys_RED_rot_CONT = ss(A_rot_cont, B_rot_cont_RED, C_rot_cont, D_rot_cont);
        sys_BLACK_rot_CONT = ss(A_rot_cont, B_rot_cont_BLACK, C_rot_cont, D_rot_cont);
        
        
        sys_RED = c2d(sys_RED_CONT, baseRate);
        sys_BLACK = c2d(sys_BLACK_CONT, baseRate);
        
        sys_RED_ROT = c2d(sys_RED_rot_CONT, baseRate);
        sys_BLACK_ROT = c2d(sys_BLACK_rot_CONT, baseRate);
        
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%% PARAMETERS I CAN CHANGE %%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    

            b = 0.20; % Initial guess for b.
            ka = 0.08; % Initial guess for ka.
            kc = 0.08; % Initial guess for kc.
                      
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%% PARAMETERS I CANNOT CHANGE %%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 

        theta_d = 30*d2r; % Angle of the docking cone.
        d = [0.16;0.542;0]; % docking position.
        d_norm = sqrt(sum(d.^2)); % Norm of the docking position.
        
        o_prime = [0;1;0]; % Orientation of the docking cone.
        o_hat_prime = o_prime/sqrt(sum(o_prime.^2)); % Making sure it is a normal vector.
        
        
    % For the max acceleration:
        a_max = 0.4/BLACKMass; % ASSUMING A 0.2N MAX
        
%% LVF DESIGN PROCEDURE:

        % For the switching condition:
            acceptableRadius = 0.05; % a 5cm radius.
            timeThreshold = 5; % 5 seconds staying in the radius
            cntThreshold = timeThreshold/baseRate;
            r_co_over_a_prime = 4/5;

        % Initial parameters for the search:
            a_prime = 0.2; % Initial guess for a
            v_max = 0.1; % initial guess for vmax
            
        % INITIALIZE PARAMETERS FOR THE LINE-SEARCH
            params = [v_max, a_prime]; % The parameter line to search.
        
        % SELECT OUR MAXIMUM ROTATION RATES ACCORDING TO THE TRAJECTORY:
            rotStuff = [abs(w_body),theta_d,d']; % The angular data for the target.
        
        % PICK OUR SEARCH DIRECTION FOR THE PARAMETER LINE-SEARCH:
            searchDirection = [0.1,0.0]; % Only allow v_max to vary.
            
        % PICK A DESIRED ACCURACY, STEP SIZE, SHRINK FACTOR.
            desiredAccuracy = 0.0001;
            stepSize = 0.001;
            stepShrinkingFactor = 0.5;
               
        % SELECT NEW PARAMETERS:
            newParams = lineSearch(params, 'a_max_LVF', searchDirection, desiredAccuracy, a_max, stepSize, stepShrinkingFactor, rotStuff);    
        
        % OUTPUT THE LYAPUNOV VECTOR FIELD PARAMETERS:
            v_max = newParams(1);
            a_prime = newParams(2);
        
        % Convert v_max into the gain:
            k_v_rel = 2*v_max/a_prime;
            r_co = r_co_over_a_prime*a_prime;
                    

%% CLVF DESIGN PROCEDURE

        % FIRST, we'll take the a_dock and o_dock (both in the body-fixed
        % frame) to create our "a" and "o_hat_B" parameters.
            
            aTimesOVec = d + a_prime*o_hat_prime; % All in the body-fixed frame.
            a = sqrt(sum(aTimesOVec.^2));
            o_hat_B = aTimesOVec./a;

        % INITIALIZE PARAMETERS FOR THE LINE-SEARCH
            params = [kc, ka, b, a]; % The parameter line to search.
        
        % SELECT OUR MAXIMUM ROTATION RATES ACCORDING TO THE TRAJECTORY:
            rotStuff = [abs(w_body),0,0]; % The angular data for the target.
            
        % BEFORE DOING THE SEARCH, CHECK IF IT IS POSSIBLE:
            disp("Our LOWEST possible:")
            disp(a_max_CLVF2D([0,0,inf,a], rotStuff))
            
            if a_max_CLVF2D([0,0,inf,a], rotStuff) > a_max
%                 error("Not a possible scenario");
            else
                disp("mkai...")
            end
        
            
        % PICK OUR SEARCH DIRECTION FOR THE PARAMETER LINE-SEARCH:
            searchDirection = [0.1,0.1,-0.5,0]; % USUAL ONE!
%             searchDirection = [0.1,0.1,0.1,0];
%             searchDirection = [1,0,0,0];
            
        % PICK A DESIRED ACCURACY, STEP SIZE, SHRINK FACTOR.
            desiredAccuracy = 0.0001;
            stepSize = 0.001;
            stepShrinkingFactor = 0.5;
        
        % SELECT NEW PARAMETERS:
            newParams = lineSearch(params, 'a_max_CLVF2D', searchDirection, desiredAccuracy, a_max, stepSize, stepShrinkingFactor, rotStuff);
            
            kc = newParams(1);
            ka = newParams(2);
            b = newParams(3);
            
            disp("ka is")
            disp(ka)
            disp("kc is")
            disp(kc)
            disp("b is")
            disp(b)
            

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

