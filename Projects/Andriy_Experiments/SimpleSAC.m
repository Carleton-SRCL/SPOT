%Runs the simplified SIM:

m = 16.95;

A = [0,1;0,0];
b = [0;1./16.95];

C = [1,0];

D = 0;

IC = [2.45;0;1.21;0;0;0];

A_big = blkdiag(A,A,A);
B_big = blkdiag(b,b,b);
C_big = blkdiag(C,C,C);
D_big = blkdiag(D,D,D);

tsim = 200;

load('model_param.mat'); 
load('thruster_param.mat');

model_param(1)                 = REDMass;
model_param(2)                 = REDInertia;
model_param(3)                 = BLACKMass;
model_param(4)                 = BLACKInertia;

% Parameters for the ideal model:
ts                             = 2.0; 
zeta                           = 1.0; 
wn                             = 4/(ts*zeta);
alpha                          = 1;
Am                             = [zeros(3,3), eye(3,3);...
                                -wn^2*eye(3,3), -2*zeta*wn*eye(3,3)];
Bm                             = [zeros(3,3);wn^2*eye(3,3)];
Cm                             = eye(6,6); 
Dm                             = zeros(6,3);

% To calculate the real ideal model output:
Cmout                          = [alpha*eye(3,3),eye(3,3)]; 

%To calculate controller to optimize:
Ceout                          = [eye(3),zeros(3,3)];

% Initial pos. & vel. of RED:
xm_ini                         = [0, 0, 0, 0, 0, 0]'; 

Force = 0.5;%Max force that can be made by the spacecraft
Force = 0.5;%Max force I'm willing to use
Radius = 0.75;%Radius to keep
Mass = model_param(1);
%Figure out the angular velocity of the figure (PURE CIRCLE!)
omega = sqrt(Force./(Mass*Radius));

t_vector = 1:0.001:tsim;
ratio = 10/7;%Ratio between R and H parameters for the cycloid thing shape.
scaling = 1/ratio * Radius;%Controls figure size. Make sure there's clearance with target in central region of figure.
R = ratio*scaling;
H = R/ratio;
% H = H*0;%One sinusoid please...
omega = 0.03;%Speed of the figure, should be slower than the reference model
% omega = 0.035
phi = 46-3.*pi./4;%seconds

[track,Force,Jerk] = cycloid(t_vector,R,H,omega,0,0,phi);

xLength = 3.5;
yLength = 2.25;
offset = [xLength./2,yLength./2];
track(:,1) = track(:,1)+offset(1);
track(:,2) = track(:,2)+offset(2);

Path_xyz.signals.values = [zeros(0,3);track(1:end-0,1:3)];
Path_xyz.signals.dimensions = [3];
Path_xyz.time = t_vector;

serverRate                     = 0.1;       % 10 Hz

sys                            = ss(Am, Bm, Cm, Dm);

opt                            = c2dOptions('Method','tustin',...
                                'FractDelayApproxOrder',3);
sysd1                          = c2d(sys,serverRate,opt);

KuI0 = zeros(3);
KeI0 = zeros(3);
KxI0 = zeros(3,6);

GeP                            = 1e-0*eye(3,3);
GxP                            = 8e-0*eye(6,6);
GuP                            = 8e-0*eye(3,3);
GeI                            = 2e-0*eye(3,3); 
GxI                            = 1e-0*eye(6,6); 
GuI                            = 1e-0*eye(3,3); 

% Set the forgetting rates. Higher values increase robustness to unknown
% disturbances:
sigma_eI                       = 0.4;
sigma_xI                       = 0.4;
sigma_uI                       = 0.4;

alpha                          = 1;

Sat = 0.5;

Sampletime = 0.01;%10 Hz

OmegaAct = 1;
