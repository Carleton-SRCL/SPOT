% The following script is the initializer for SPOT 2.0; in this script,
% users define all initials parameters and/or constants required for
% simulation and experiment.

% Version: 3.06 (Beta Release)

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
fprintf('|Current Version: 3.06 (Beta Release)                            |\n')
fprintf('|                                                                |\n')
fprintf('|Last Edit: 2021-03-02                                           |\n')
fprintf('|                                                                |\n')
fprintf('|----------------------------------------------------------------|\n')
fprintf('|----------------------------------------------------------------|\n')

%% User-defined constants:

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

%% SELECT YOUR INITIAL CONDITIONS SET:
%Load nonlinear optimized gains, only for the searches considered:
% Opt = load('2020Optimizations200Seconds.mat','DE','SADE','PSO','SPSO');
% Opt = load('4040Optimizations400Seconds.mat','DE','SADE','PSO','SPSO');%This one actually works!!!!!!!!!!!
% Opt = load('Test2Optimizations400Seconds.mat','DE','SADE','PSO','SPSO');
% Opt = load('TEST.mat','DE','SADE','PSO','SPSO');
% Opt = load('TestFFOptimizations400Seconds.mat');
Glow = load('TestFFOptimizations400SecondsWOUTP.mat');

%Easiest way to switch between controllers:
% Gains = 'DE';
Gains = 'SADE';
% Gains = 'PSO';
Gains = 'SPSO';
Gains = 'Hand';

%hack:
% Opt.DE = DE;

q = 1.*eye(3);
r = 10.*eye(3);

%Cost function variables

% Parameters for the ideal model:
ts                             = 2; 
zeta                           = 0.6; 
wn                             = 4/(ts*zeta);
alpha                          = 1;
Am                             = [zeros(3,3), eye(3,3);...
                                -wn^2*eye(3,3), -2*zeta*wn*eye(3,3)];
Bm                             = [zeros(3,3);wn^2*eye(3,3)];
Cm                             = eye(6,6); 
Dm                             = zeros(6,3);
%SISO version:
Model.A                             = [0, 1;...
                                -wn^2*1, -2*zeta*wn*1];
Model.B                             = [0;wn^2*1];
Model.C                             = [1,0]; 
Model.D                             = 0;

% To calculate the real ideal model output:
Cmout                          = [alpha*eye(3,3),eye(3,3)]; 

%To calculate controller to optimize:
Ceout                          = [eye(3),zeros(3,3)];

% Initial pos. & vel. of RED:
xm_ini                         = [xLength/2+0.7, yLength/2, 0, 0, 0, 0]'; 
                
%This script was already being used by OptimizationPlots
SetGains

Cmout                          = [alpha*eye(3,3),eye(3,3)];

Cmout                          = [eye(3,3),zeros(3,3)];

%Create the PID and LQR gains:
% PID:
K_Force = 1;
Kd_Force = 3;
%LQR Controller:
% load('model_param.mat'); 

A1 = [0,1;
     0,0];
B1 = [0;
    1/BLACKMass];
Plant.A = A1;
Plant.B = B1;
Plant.C = Model.C;
Plant.D = Model.D;
Q1 = [10,0;
     0,10];
R1 = [1];
[X1,L1,G1] = care (A1,B1,Q1,R1); %Awesome for 1 axis

%3 axis now!:
A3 = blkdiag(A1,A1,A1);
B3 = blkdiag(B1,B1,B1);
Q3 = blkdiag(Q1,Q1,Q1);
R3 = blkdiag(R1,R1,R1);
[X3,L3,G3] = care(A3,B3,Q3,R3);

%Is perfect tracking... wrong?
%Check with lsims:
modelss = ss(Model.A,Model.B,[1,0],Model.D);
SACss = ss(Model.A,Model.B,[-0.4934,-4.9336],0.4934);
plantss = ss(Plant.A,Plant.B,Plant.C,Plant.D);

%Create "Perfect tracking" situation:
OLss = series(SACss,plantss);
% step(OLss)
% grid on
% hold on
% step(modelss)
%Oh look, they're literally the exact same.
%I don't know what to believe anymore...

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

Phase0_Duration                = 10;        % [s]
Phase1_Duration                = 5;         % [s]
Phase2_Duration                = 20;        % [s]
Phase3_Duration                = 211;       % [s]
Phase4_Duration                = 20;        % [s]
Phase5_Duration                = 5;         % [s]

% Set the duration of the sub-phases. Sub-phases occur during the
% experiment phase (Phase3_Duration) and must be manually inserted into the
% diagram. The total duration of the sub-phases must equal the length of
% the Phase3_Duration.

Phase3_SubPhase1_Duration      = 5;        % [s]
Phase3_SubPhase2_Duration      = 5;        % [s]
Phase3_SubPhase3_Duration      = 1;        % [s]
Phase3_SubPhase4_Duration      = 200;       % [s]

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
Phase3_SubPhase3_End           = Phase2_End + Phase3_SubPhase1_Duration + ...
                                 Phase3_SubPhase2_Duration +...
                                 Phase3_SubPhase3_Duration;
Phase3_SubPhase4_End           = Phase2_End + Phase3_SubPhase1_Duration + ...
                                 Phase3_SubPhase2_Duration +...
                                 Phase3_SubPhase3_Duration +...
                                 Phase3_SubPhase4_Duration;                                           
                         
%%  Set the drop, initial, and home positions for each platform:


drop_states_RED           = [2.60644311523438,1.21791186523438,3.08142089843750]; % [m; m; rad]
drop_states_BLACK         = [2.3,1.5,0.176047682762146];      % [m; m; rad]
drop_states_BLUE          = [ xLength/2+0.9; yLength/2+0.5; 0];         % [m; m; rad]

init_states_RED           = [ xLength/2+0.7; yLength/2; 0]; % [m; m; rad]
init_states_BLACK         = [ xLength/2; yLength/2; 0];      % [m; m; rad]
init_states_BLUE          = [ xLength/2+0.9; yLength/2+0.5; 0];      % [m; m; rad]

home_states_RED           = [ xLength/2+0.7; yLength/2; 0]; % [m; m; rad]
home_states_BLACK         = [ xLength/2; yLength/2; 0];      % [m; m; rad]
home_states_BLUE          = [ xLength/2-0.9; yLength/2+0.5; 0];  % [m; m; rad]

%% PARAMETERS AND INITIAL CONDITIONS  
%Change the initial position so it starts on the track:
SineFreq = 0.05;%Frequency, in rad/s of sinusoids
SineAmp = 1;%meter.

%Chaser relative position command:

Force = 0.5;%Max force that can be made by the spacecraft
Force = 0.5;%Max force I'm willing to use
Radius = 0.5;%Radius to keep
Mass = model_param(3);
%Figure out the angular velocity of the figure (PURE CIRCLE!)
omega = sqrt(Force./(Mass*Radius));
rotation = 2*pi/6+0.15;%Rotate the whole figure

t_vector = 1:0.001:tsim;
ratio = 10/7;%Ratio between R and H parameters for the cycloid thing shape.
scaling = 1/ratio * Radius;%Controls figure size. Make sure there's clearance with target in central region of figure.
R = ratio*scaling;
H = R/ratio;
%H = H*0;%One sinusoid please...
omega = 0.03;%Speed of the figure, should be slower than the reference model
omega = 0.06;%Used for the LQR test, maybe...
omega = 0.03;

phi = 30;
offset = init_states_BLACK;%Center of the table
[track,Force,Jerk] = cycloid(t_vector,R,H,omega,0,0,phi,rotation);
% close 10
track(:,1) = track(:,1)+offset(1);
track(:,2) = track(:,2)+offset(2);

Path_xyz.time = t_vector;
delaycount = 100000*0;

% %Searching for a command path that fits on the table:

% figure(10)
% plot(track(1:10000,1),track(1:10000,2))
% grid on
% axis equal
% hold on
% plot(track(10001:100000,1),track(10001:100000,2),'r')
% ylim([0,yLength])
% xlim([0,xLength])

Path_xyz.signals.values = [zeros(delaycount,3);track(1:end-delaycount,1:3)];
Path_xyz.signals.dimensions = [3];
PathAtStart = Path_xyz.signals.values(Path_xyz.time == Phase1_End,:);
Path_xyz.signals.values = [zeros(delaycount,3);track(1:end-delaycount,1:3)];
Path_xyz.signals.dimensions = [3];

FirstPosition = Path_xyz.signals.values(find(t_vector>Phase3_SubPhase3_End,1),:)';
LastPosition = Path_xyz.signals.values(find(t_vector>Phase3_SubPhase4_End,1),:)';

init_states_BLACK = FirstPosition;
home_states_BLACK = LastPosition;      % [m; m; rad]
%No, Initial pos. & vel. of BLACK:
xm_ini                         = [FirstPosition(1), FirstPosition(2), 0, 0, 0, 0]'; 


%% Parameters added by Andriy Predmyrskyy for SAC search methods:
%Constructing the time history to be used in simulation:

% 
% LQR.TargetPos = TargetPos
% LQR.ThrustOut = ThrustOut
% save('LQRAndPID.mat')

%Creating Two other Time Histories:

%Square (Step Commands)
% DeltaT = 400;%Seconds
% Amp = 1;
% track = [Amp*round(mod(t_vector,DeltaT)./DeltaT);
%     Amp*round(mod(t_vector+DeltaT./4,DeltaT)./DeltaT);
%     zeros(size(t_vector))];
% track = track';
% track(:,1) = track(:,1)+offset(1);%Keep offsets
% track(:,2) = track(:,2)+offset(2);


%NOTE! COME UP WITH A PATH THAT'S A SQUARE BUT DOESN'T DO IT IN DISCRETE
%STEP CHUNKS!!!

%Testing something out:
Gamma = blkdiag(GeP(1,1), GxP(1,1),GxP(4,4), GuP(1,1), GeI(1,1), GxI(1,1),GxI(4,4), GuI(1));
% SACSim(Plant,Model,Path_xyz.time,Path_xyz.signals.values(:,1),Gamma)

%%Stick all the stuff about the feedforward parallelization (read: PID sac)
%Find the set of PD controllers with HIGH gain, but definitely stable for a
%range of masses (say 10-20 Kg):
pole =1./1;
Kd_sac = 100;
Kp_sac = 1./0.1;
% Kp_sac = 3.1623;
% pole = Kp_sac./20;
% pole = 0.3371;
% Kp_sac = 3.1623;
mass2 = 20;
% Kd_sac = G1(2)*10;
% Kp_sac = G1(1)*10;
roots([mass2,Kd_sac,Kp_sac])
% %Set up the root locus
% PDController = tf([1,pole],1);
% plant = tf(1,[mass2,0,0]);
% sysModel = series(PDController,plant);
% rlocus(sysModel)

%Seems it's stable for all gains?
%So then the choice of Kd and Kp will come down to steady state
%performance.

%Create discrete Ideal model and discrete inverse controllers:
% Take the continuous state-space model and convert it to discrete time,
% knowing the rate at which data will be read:
sys                            = ss(Am, Bm, Cm, Dm);

opt                            = c2dOptions('Method','tustin',...
                                'FractDelayApproxOrder',3);
sysd1                          = c2d(sys,baseRate,opt);

%Create the inverse controller (The bit that creates the augmented system)
ModelFilter = tf([1./Kp_sac],[1./pole,1])
[SSModelFilter.A,SSModelFilter.B,SSModelFilter.C,SSModelFilter.D] = tf2ss(ModelFilter.Numerator{:},ModelFilter.Denominator{:});
InverseFilter.A = blkdiag(SSModelFilter.A,SSModelFilter.A,SSModelFilter.A);
InverseFilter.B = blkdiag(SSModelFilter.B,SSModelFilter.B,SSModelFilter.B);
InverseFilter.C = blkdiag(SSModelFilter.C,SSModelFilter.C,SSModelFilter.C);
InverseFilter.D = blkdiag(SSModelFilter.D,SSModelFilter.D,SSModelFilter.D);
sysInverse                     = ss(InverseFilter.A,InverseFilter.B,InverseFilter.C,InverseFilter.D);
sysd2                          = c2d(sysInverse,baseRate,opt);


xm2_ini = -(sysd2.D(1,1).*xm_ini(1:3))./(sysd2.C(1,1)); %Sets initial output to zero. Just in case.
modeldisc = c2d(ss(Model.A,Model.B,Model.C,Model.D),0.000001,opt);
plantdisc = c2d(ss(Plant.A,Plant.B,Plant.C,Plant.D),0.000001,opt);
% [a,b,c,d] = LinearSACConverges(plantdisc,modeldisc);%-\(tsu)/- There's some shenanigans going on with the discrete case...
%Change the model (For sensitivity analysis):
%Note this is done AFTER LQR formulation for fairness
PerturbedMass = BLACKMass.*1;
model_param(3) = PerturbedMass;
BLACKMass = model_param(3);


%%Include the fan Disturbance Model:
%%All of this is repeated in the actual disturbance model in the sim, for
%%brevity XD

FanConeAngle = 15*d2r;
FanV0 = 10;%m/s
FanRho = 1.225;%kg/m3
FanDiam= 0.3;%m, 30 centimeters seems normal
Fanr0 = FanDiam/2;
Cd45 = 0.8;%Coefficient of drag for sideways, wikipedia
Cd90 = 1.05;%face on, wikipedia
Asat = 0.3*0.3;%m2, Area of the satellite, shown to the fan. No clue. FIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIX
FANPOSITION = [1.75;-0.75];%Static Position of the fan,m, Whatever reference system the simulation uses.


%Function for velocity at a given distance:
FanVatX = @(x) FanV0 * ((Fanr0).^2)./((Fanr0+tan(FanConeAngle)*x).^2);
Fd = @(x)1./2 * FanRho * FanVatX(x).^2*Cd90*Asat;
%Fd_dot Might as well just calculate it...
%%Adding the disturbance component of SAC. This is gonna be really cool if
%%it works...

GammaDi = eye(2);
FANPOSITIONGUESS = 0*[1.75;-0.75];
L = eye(2);
tau = 2;%Time constant for filters (Gets rid of force command noise -.-


%Developing the linear disturbance compensator for SAC to take advantage
%of: should be able to use a version that approximates. SAC adaptation
%should be able to handle the varying components pretty quickly.

%Perturbation depends on position, so we have a linear system that
%replicates the position:
K = 3;%This is, just, part of the thing:
%So we have the following two sinusoids managing position:

GzI = 1e0.*eye(12);
KzI0 = zeros(3,12);
GzP = 0e-2.*eye(12);
sigma_zI = 0;

Kd = eye(3); %Stabilizing DAC disturbance model gain.

%I give up, we're generating a disturbance that we can for sure take out...

phase1off = 2*pi + 3-2+1.0654;
phase2off = mod(pi^66,2*pi);
omega1off = 0.03;
omega2off = 0.06;
Amp1off = 0.2;
Amp2off = 0.1;

%Which adaptive gains to use:
% Initialize the adaptive gains:
%Consider solving for the steady state gains of the discrete versions of
%the systems instead, actually:

KeI0           = 0.*[   0.1063   -0.0058         0
                            -0.0058    0.0003         0
                             0         0              0 ];
                         
KxI0           = 0.*[-0.2193*eye(3),-3.2891*eye(3)];
                 
KuI0           = 0.*0.2193*[1,0,0;0,1,0;0,0,0];

%Analyzing the LQR tf to beat:
LQROL = series(tf(G1([2,1]),1),tf(1,[BLACKMass,0,0]));
LQRCL = feedback(LQROL,1);
% figure
% step(LQRCL)
% grid on
% hold on
% step(tf(wn*wn,[1,2*zeta*wn,wn*wn]))

%% Start the graphical user interface:

run('GUI_v3_06');

