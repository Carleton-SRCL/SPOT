function ICStructure = getICStructure()
%% Simply meant to namespace the initial condition code.

d2r = pi/180;

% Redefine lengths of the table for simplicity:
xLength                        = 3.51155;   % [m]
yLength                        = 2.41935;   % [m]

% Define the different starting rotations:
n_rotations = 5;
rotations = linspace(0, 2*pi*(1 - 1/n_rotations), n_rotations);

% Setup our various initial condition options:
ICStructure.w_body = {};
ICStructure.rT_I0 = {};
ICStructure.rC_I0 = {};


%% IC 1:
% Spin rate of the spacecraft:
ICStructure.w_body{1} = 7.0 * d2r;

% Initial position of the Target:
ICStructure.rT_I0{1} = [...
    yLength/2;...
    yLength/2;...
    rotations(1)...
]; % [m ; m ; rads]

% Initial position of the Chaser:
ICStructure.rC_I0{1} = [0.9*xLength;yLength/2-0.5;pi/2]; % [m ; m ; rads]

%% IC 2:
% Spin rate of the spacecraft:
ICStructure.w_body{2} = -8.5 * d2r;

% Initial position of the Target:
ICStructure.rT_I0{2} = [...
    yLength/2;...
    yLength/2;...
    rotations(2)...
]; % [m ; m ; rads]

% Initial position of the Chaser:
ICStructure.rC_I0{2} = [0.9*xLength;yLength/2+0.5;pi/2]; % [m ; m ; rads]

%% IC 3:
% Spin rate of the spacecraft:
ICStructure.w_body{3} = 8.0 * d2r;

% Initial position of the Target:
ICStructure.rT_I0{3} = [...
    yLength/2;...
    yLength/2;...
    rotations(3)...
]; % [m ; m ; rads]

% Initial position of the Chaser:
ICStructure.rC_I0{3} = [0.9*xLength;yLength/2-0.5;pi/2]; % [m ; m ; rads]

%% IC 4:
% Spin rate of the spacecraft:
ICStructure.w_body{4} = -6.0 * d2r;

% Initial position of the Target:
ICStructure.rT_I0{4} = [...
    yLength/2;...
    yLength/2;...
    rotations(4)...
]; % [m ; m ; rads]

% Initial position of the Chaser:
ICStructure.rC_I0{4} = [0.9*xLength;yLength/2+0.5;pi/2]; % [m ; m ; rads]

%% IC 5: 
% Spin rate of the spacecraft:
ICStructure.w_body{5} = 6.0 * d2r;

% Initial position of the Target:
ICStructure.rT_I0{5} = [...
    yLength/2;...
    yLength/2;...
    rotations(5)...
]; % [m ; m ; rads]

% Initial position of the Chaser:
ICStructure.rC_I0{5} = [0.9*xLength;yLength/2-0.5;pi/2]; % [m ; m ; rads]

end

