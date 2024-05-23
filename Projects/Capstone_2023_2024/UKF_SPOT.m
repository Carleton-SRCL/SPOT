%% Spacecraft Data
    m = 11.210; % kg  
    J = 0.20184; % kgm^2
    dt = 0.01; % seconds
    N = 6; % Number of states
    method = 1; % Method of NSPDM

    %% Kalman Filter setup
    x = [1.00034654284122e-06	1.00000000000000e-06	3.63628392455557e-06	0.00184501303471272	5.27091972607854e-05	0.000199668122223857	0.1	1.00000000000000e-06	0.162242557505519	0.134194988051565];
    % State transition matrix
    A = [1 0 0 dt 0 0;
         0 1 0 0 dt 0;
         0 0 1 0 0 dt;
         0 0 0 1 0 0;
         0 0 0 0 1 0;
         0 0 0 0 0 1];
    % sensor data
    H = [1 0 0 0 0 0;
         0 1 0 0 0 0;
         0 0 1 0 0 0];

%% Black
Q_Black = [0.0000005 0 0 0 0 0;
               0 0.0000005 0 0 0 0;
               0 0 0.00005 0 0 0;
               0 0 0 0.00005 0 0;
               0 0 0 0 0.00005 0;
               0 0 0 0 0 0.00005];;
variance = 0.99;
R_Black = [variance 0 0;
    0 variance 0;
    0 0 variance];
alpha_Black = 0.9; % very small var, never > 1
beta_Black = 2; % 2 for gaussian
kappa_Black = 20; % Starts at zero, usually small
lamda_Black = (alpha_Black^2)*(N + kappa_Black) - N; % N is the number of dimensions
P_IC_Black = 0.2*eye(6); %% Change this

thresh_Black = 7.815;
thresh_Factor_Black = 0.1;

%% Blue

Q_Blue = [0.000075 0 0 0 0 0;
               0 0.000075 0 0 0 0;
               0 0 0.00005 0 0 0;
               0 0 0 0.00005 0 0;
               0 0 0 0 0.00005 0;
               0 0 0 0 0 0.00005]; %% Change this
variance = 0.01;
R_Blue = [variance 0 0;
    0 variance 0;
    0 0 variance];

alpha_Blue = 0.9; % very small var, never > 1
beta_Blue = 20; % 2 for gaussian
kappa_Blue = 10; % Starts at zero, usually small
lamda_Blue = (alpha_Blue^2)*(N + kappa_Blue) - N; % N is the number of dimensions
P_IC_Blue = 0.2*eye(6); %% Change this

thresh_Blue = 2;
thresh_Factor_Blue = 0.5;
