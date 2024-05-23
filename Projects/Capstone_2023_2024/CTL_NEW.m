%% Constants
m = mRED; % kg
J = IRED; % kgm^2
F_max = 0.22; % N
T_max = 0.0235;% Nm
a = 25.028; % Value for low pass filter

%% Roaming Constants
Time_Look = 25; % seconds
Hz = 0.01;
Red_Rate = 0.025;

%% Bad PD
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

%% CTL LQR
    %% Setting up dynamics of system
    % Method 1 based off of PD damping ratio
    % w = 3.5;
    % wd = 16*w^(2);
    % t = 1;
    % td = 20*t^(1/2);
    % r = 1;
    % Method 2 based off of Sat damping ratio
    w = 15;
    wd = 30*w^(2);
    t = 1;
    td = 20*t^(1/2);
    r = 1;

  % Tuned for best tracking
    % w = 9138.41131640842;
    % wd = w;
    %  t = 1.00000117521453;
    % td = t;
    % r = 9.99998843608298;

    A = [0 0 0 1 0 0;
        0 0 0 0 1 0;
        0 0 0 0 0 1;
        0 0 0 0 0 0;
        0 0 0 0 0 0;
        0 0 0 0 0 0];

    B1 = [0 0 0;
        0 0 0;
        0 0 0;
        1/m 0 0;
        0 1/m 0;
        0 0 1/J];

    % Weighting matrix
    Q = [w 0 0 0 0 0;
        0 w 0 0 0 0;
        0 0 t 0 0 0;
        0 0 0 wd 0 0;
        0 0 0 0 wd 0;
        0 0 0 0 0 td];

    R = [r 0 0;
        0 r 0;
        0 0 r];

    % Solving cost eq
    k = lqr(A,B1,Q,R);

%% CTL DAC Gains
k_1_0 = k(1,1);
k_1_t_0 = k(3,3);
k_2_0 = k(1,4);
k_2_t_0 = k(3,6);

lambda1 = 0.4;
lambda2 = 0.4;
lambda12 = 0.4;
p = 0.4;

lambda1_t = 0.05;
lambda2_t = 0.05;
lambda12_t = 0.05;
p_t = 0.05;


%% Filter
delta = 2; % offset from body to camera frame (deg)
UKF_SPOT