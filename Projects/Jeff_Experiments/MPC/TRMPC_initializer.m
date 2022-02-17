% Just messing around to get the TRMPC working...
% IF we assume that "W" can occur in arbitrary direction, problem reduces
% to finding eigenvalues of A, and then checking (lambda_max)^s < 0.01

% NOTE - THE STATES WILL BE MODELLED AS [POS_VECTOR; VEL_VECTOR]

%% simple constants:
d2r = pi/180;
r2d = 1/d2r;

%% Simulation meta-data:

% control frequency in Hz:
freq_control = 20;

% Set the step size of the simulation:
FixedStep = 1/freq_control;

% For now, leaving the control step the same as the dynamics step.
control_freq = 5;
ControlStep = 1/control_freq;

% Set the simulation stop time:
T = 1000;

% Pick the horizon for the MPC:
Np = 15;

% Create a time vector for future prediction:
future_time_vector = (1:Np) * ControlStep;

%% Hill's dynamics:

RE = 6378;
MU = 3.986 * 10^5;
J2 = -1.08264*10^-3;

% First, we'll set up a target orbit:
semiMajorAxis = 7500;
e = 0.01;
i = 0 * d2r;
RAAN = 0*d2r;
argOfPerigee = 0*d2r;
trueAnomaly = 0*d2r;
    
% Get the initial position and velocity from the orbital elements:
[RT_I0, VT_I0] = posAndVelFromOEs(semiMajorAxis, e, i, RAAN, argOfPerigee, trueAnomaly, MU);

%% Next, setup the chaser:
rC_I0 = [30;0;0]*10^-3;
vC_I0 = [0;0;0]*10^-3;

RC_I0 = rC_I0 + RT_I0;
VC_I0 = vC_I0 + VT_I0;

% chaser mass [kg]
m = 10; 

%% Hill's dynamics:

n = sqrt(MU/semiMajorAxis^3);

BL = [3*n^2      0       0;
     0          0       0;
     0          0    -n^2];
 
BR = [0         2*n     0;
      -2*n      0       0;
      0         0       0];
 
A_c = [zeros(3), eye(3);
      BL        BR    ];
  
B_c = [zeros(3);eye(3)/m];

C = eye(6);
D = zeros(6,3);

% Convert our own way:
%A_d = eye(size(A_c)) + (A_c*ControlStep) + (A_c*ControlStep)^2/2 + (A_c*ControlStep)^3/6 + (A_c*ControlStep)^4/24;
%B_d = A_c\(A_d - eye(size(A_c)))*B_c;

sys = ss(A_c, B_c, C, D);
sys_d = c2d(sys, ControlStep);

A_d = sys_d.A;
B_d = sys_d.B;

%% Build the equality constraints (based on dynamics):
Aeq = return_equality_mat(A_d, B_d, Np);

%% Select an LQR controller

R = 1*eye(3);
Q = diag([1, 1, 1, 1, 1, 1])*100;

% Factor of extra weight for the final step:
final_weight_factor = 100;


%% Build the cost matrix (based on LQR gains):
Q = return_cost_matrix(R, Q, Np, final_weight_factor*Q);

%% Describing the INPUT constraints.
% NOTE - this will now be incorperated as a LB-UB combo.
u_con_mat = [];
u_max_scalar = 1; % [m/s^2]
u_con_vec = [];

%% Set up the state constraints:
% Only a 4x6 for this one:
x_con_mat = zeros(4, 6);

% The location of the docking port:
d = [3.0 ; 0 ; 0];
d_hat = d./sqrt(sum(d.^2));

% Choose some random rotation matrix for the cone (use this to get o_hat in
% CLVF)
C_CB = C3(pi/6);
%C_CB = eye(3);
o_hat_prime = C_CB' * d_hat;

% The matrix and vector describing the cone:
[A_cone, b_cone] = return_square_cone(pi/6, d,  C_CB);

% Draw the cone to make sure.
% [CONFIRMED] gives us the cone that we want.
% draw_cone(A_cone, b_cone, [-6,15],[-20,20],[-20,20],1);

% the maximum speed [NOTE - THIS WILL NOW BE INCORPERATED AS LB-UB COMBO]:
v_max = 1;

% The actual state-constraint vector:
x_con_vec = b_cone;

%% Create the LB and UB vectors:
% input and state lower bounds and upper bounds:
u_ub = [u_max_scalar;u_max_scalar;u_max_scalar];
u_lb = -u_ub;

x_ub = [Inf; Inf; Inf; v_max ; v_max ; v_max];
x_lb = -x_ub;

lb_vector = [u_lb ; x_lb];
ub_vector = [u_ub ; x_ub];

[LB, UB] = return_lb_and_ub_vectors(lb_vector, ub_vector, Np);

%% Describing the target dynamics:
% Pick a gamma angle:
gamma = 30*pi/180;

% Pick inertias and transverse angular velocity:
J_transverse = 2;
J_z = 5;
w_t = 5 * d2r;

% Calculate remaining values:
h_t = J_transverse * w_t;
h = h_t / sin(gamma);
h_z = h * cos(gamma);
w_z = h_z / J_z;

% select the initial mu and theta angles:
mu0 = 0;
theta0 = 0;

%% Some final dimensionality things:

% Size of total vector per step:
dimPerStep = numel(u_ub) + numel(x_ub);

% The total dimension of the X-vector:
X_SIZE = (dimPerStep) * Np;

% Total number of equality contraints:
N_EQ = Np*size(A_d, 1);

% Number of inequality constraints on the inner MPC:
N_INEQ_INNER = Np * (size(u_con_mat, 1) + size(x_con_mat, 1));

% Get the number of constraints in the outer (Just the plane constraint @ each step).
N_INEQ_OUTER = Np;

% Initial guess for the MPC
X0 = zeros(X_SIZE,1);

% Build the warm-start matrix:
warm_start_matrix = return_warm_start_matrix(dimPerStep, Np);

%% Set the tracking point for the outer MPC:
% The outer radius of safety:
r_safe = 10; 

% The tracking point:
tracking_point = get_tracking_point(o_hat_prime, d, r_safe);

%% Setup the switching conditions:
acceptableAngle = 0.05;
acceptableDistance = 0.1;
cntThreshold = 10;

%% Set up the simulation, and run!
set_param('tumblingExample','StopTime',num2str(T),'FixedStep',num2str(FixedStep));
simOut = sim("tumblingExample");





