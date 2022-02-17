function MPCStructure = getMPCStructure(m, b_cone, R, Q, Q_final, u_max_scalar, v_max, tracking_point)
% CHOOSE THE STEP HORIZON:
Np = 5;
MPCStructure.Np = Np;

% CHOOSE THE CONTROL STEP:
ControlFrequency = 20;
MPCStructure.ControlStep = 1/ControlFrequency;

% Now, get the future_time_vector:
MPCStructure.future_time_vector = (1:Np) * MPCStructure.ControlStep;

A_c = [zeros(3), eye(3);
      zeros(3,6)       ];

B_c = [zeros(3);eye(3)/m];

C = eye(6);
D = zeros(6,3);

sys = ss(A_c, B_c, C, D);
sys_d = c2d(sys, MPCStructure.ControlStep);

A_d = sys_d.A;
B_d = sys_d.B;
b_cone = b_cone;

% Store into the MPC structure.
MPCStructure.A_d = A_d;
MPCStructure.B_d = B_d;

% GET THE EQUALITY MATRIX:
MPCStructure.Aeq = return_equality_mat(A_d, B_d, Np);

%% Build the cost matrix (based on LQR gains):
MPCStructure.Q = return_cost_matrix(R, Q, Np, Q_final);

%% Describing the INPUT constraints.
% NOTE - this will now be incorperated as a LB-UB combo.
MPCStructure.u_con_mat = [];
MPCStructure.u_max_scalar = u_max_scalar; % [m/s^2]
MPCStructure.u_con_vec = [];

%% Set up the state constraints:
% Only a 4x6 for this one:
MPCStructure.x_con_mat = zeros(4, 6);

% The actual state-constraint vector:
MPCStructure.x_con_vec = b_cone;

%% Create the LB and UB vectors:
% input and state lower bounds and upper bounds:
u_ub = [u_max_scalar;u_max_scalar;u_max_scalar];
u_lb = -u_ub;

x_ub = [Inf; Inf; Inf; v_max ; v_max ; v_max];
x_lb = -x_ub;

lb_vector = [u_lb ; x_lb];
ub_vector = [u_ub ; x_ub];

[MPCStructure.LB, MPCStructure.UB] = return_lb_and_ub_vectors(lb_vector, ub_vector, Np);

%% Some final dimensionality things:

% Size of total vector per step:
dimPerStep = numel(u_ub) + numel(x_ub);

% The total dimension of the X-vector:
MPCStructure.X_SIZE = (dimPerStep) * Np;

% Total number of equality contraints:
MPCStructure.N_EQ = Np*size(A_d, 1);

% Number of inequality constraints on the inner MPC:
MPCStructure.N_INEQ_INNER = Np * (size(MPCStructure.u_con_mat, 1) + size(MPCStructure.x_con_mat, 1));

% Get the number of constraints in the outer (Just the plane constraint @ each step).
MPCStructure.N_INEQ_OUTER = Np;

% Initial guess for the MPC
MPCStructure.X0 = zeros(MPCStructure.X_SIZE,1);

% Build the warm-start matrix:
MPCStructure.warm_start_matrix = return_warm_start_matrix(dimPerStep, Np);

%% MAKE THE TRACKING POINT SLIGHTLY FURTHER OUT (TO HELP WITH THE PLANE CONSTRAINT):
MPCStructure.tracking_point = 1.02*tracking_point;

end