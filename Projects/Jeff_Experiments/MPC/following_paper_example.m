% This script follows along the example paper from 2008 or something, where
% the final set for tracking is ACTUALLY SHOWN AND SOMEWHAT EXPLAINED.

clear; close all; beep off;

% Set some discrete dynamics matrices:
A = [1 1;0 1];
B = [0 0.5;1 0.5];
C = [1 0];
D = [0 0];

% Pick some weight matrices, solve the lqr:
Q = eye(2);
R = eye(2);
sys = ss(A, B, C, D, 1);
[K, P, ~] = lqr(sys, Q, R);

% Kalman gain appears negative.
K = -K;

% Define the M_theta and N_theta matrices:
M_theta = [1 0 0 0;0 1 1 -2]';
N_theta = [1 0];

% Define the matrix "L":
L = [-K eye(2)]*M_theta;

% Set up the Aw matrix:
Aw = [A+B*K, B*L; zeros(2,2), eye(2)];

