% Finds the centre of mass of the platform. Collect data using
% auto_calibrate.exe

clear
clc
close all

dataType = 1; % 1 = use real data; 2 = use simulated data

%% actual data
if dataType == 1
    r_21 = [289;3];
    r_32 = [0;-248];
    r_43 = [-288;0];
    r_14 = [-1;245];
    r_24 = [288;248];
    r_31 = [289;-245];
    
    % Calculates the MOI of SPOT using ground truth data
    %filename = 'good_spin_data.txt'; % data file in same directory as this .m file
    filename = 'long_spin_3.txt';
    columns = 13; % how many columns are in the text file
    fileID = fopen(filename); % opening the file
    
    data = fscanf(fileID,'%f',[columns,Inf])'; % getting the data
    
    t = data(:,1)';
    r_1x = data(:,8)'; % top left
    r_1y = data(:,9)';
    r_2x = data(:,11)';% top right
    r_2y = data(:,12)';
    r_3x = data(:,2)'; % bottom right
    r_3y = data(:,3)';
    r_4x = data(:,5)'; % bottom left
    r_4y = data(:,6)';
    
    timesteps = t;
    
    %% Filtering raw data
    F_c = 0.5; % [Hz] cutoff frequency
    N = 4; % order of the butterworth filter
    
    Fs = 20; % [Hz]
    f_c = F_c/(Fs/2);
    
    [b,a] = butter(N,f_c);
    
%     plot(r_1x);
%     hold on
    r_1x = filter(b,a,r_1x);
    r_1y = filter(b,a,r_1y);
    r_2x = filter(b,a,r_2x);
    r_2y = filter(b,a,r_2y);
    r_3x = filter(b,a,r_3x);
    r_3y = filter(b,a,r_3y);
    r_4x = filter(b,a,r_4x);
    r_4y = filter(b,a,r_4y);
%     plot(r_1x)
%     return
    
end
%% Simulated Data
if dataType == 2
    r_21 = [300;0];
    r_32 = [0;-300];
    r_43 = [-300;0];
    r_14 = [0;300];
    r_24 = [300;300];
    r_31 = [300;-300];
    
    
    rate = 0.5; % [rad/s]
    offset = [100;100]'; % centre of mass initial position in F_I
    velocity = [14; -6]; % [mm/s] in Ix and Iy
    acceleration = [10;22]; % [mm/s^2] in Ix and Iy
    
    r_1com = [-100;250];
    r_2com = [200;250];
    r_3com = [200;-50];
    r_4com = [-100;-50];
    
    r1angle = atan2(r_1com(2),r_1com(1));
    r2angle = atan2(r_2com(2),r_2com(1));
    r3angle = atan2(r_3com(2),r_3com(1));
    r4angle = atan2(r_4com(2),r_4com(1));
    
    timesteps = 0:1/10:10;
    t = timesteps;
    angles = timesteps*rate;
    
    accel_x = offset(1) + velocity(1)*t + 0.5*acceleration(1)*t.^2;
    accel_y = offset(2) + velocity(2)*t + 0.5*acceleration(2)*t.^2;
    
    r1 = [cos(angles+r1angle)*norm(r_1com); sin(angles+r1angle)*norm(r_1com)] + [accel_x; accel_y];
    r2 = [cos(angles+r2angle)*norm(r_2com); sin(angles+r2angle)*norm(r_2com)] + [accel_x; accel_y];
    r3 = [cos(angles+r3angle)*norm(r_3com); sin(angles+r3angle)*norm(r_3com)] + [accel_x; accel_y];
    r4 = [cos(angles+r4angle)*norm(r_4com); sin(angles+r4angle)*norm(r_4com)] + [accel_x; accel_y];
    
    
    % Extracting data into its components
    r_1x = r1(1,:);
    r_1y = r1(2,:);
    r_2x = r2(1,:);
    r_2y = r2(2,:);
    r_3x = r3(1,:);
    r_3y = r3(2,:);
    r_4x = r4(1,:);
    r_4y = r4(2,:);
end

%% Plotting raw data
figure(1)
plot(r_1x,r_1y,r_2x,r_2y,r_3x,r_3y,r_4x,r_4y)
axis equal
legend('LED1','LED2','LED3','LED4')
title('Raw marker data');
xlabel('Table X [mm]');
ylabel('Table Y [mm]');

%% Calculating the attitude of the platform


for i = 1:length(timesteps) % for each moment in time
    A = [r_21(1), -r_21(2);...
        r_21(2),  r_21(1);...
        r_32(1), -r_32(2);...
        r_32(2),  r_32(1);...
        r_43(1), -r_43(2);...
        r_43(2),  r_43(1);...
        r_14(1), -r_14(2);...
        r_14(2),  r_14(1);...
        r_24(1), -r_24(2);...
        r_24(2),  r_24(1);...
        r_31(1), -r_31(2);...
        r_31(2),  r_31(1)];
    
    b = [r_2x(i) - r_1x(i);...
        r_2y(i) - r_1y(i);...
        r_3x(i) - r_2x(i);...
        r_3y(i) - r_2y(i);...
        r_4x(i) - r_3x(i);...
        r_4y(i) - r_3y(i);...
        r_1x(i) - r_4x(i);...
        r_1y(i) - r_4y(i);...
        r_2x(i) - r_4x(i);...
        r_2y(i) - r_4y(i);...
        r_3x(i) - r_1x(i);...
        r_3y(i) - r_1y(i)];
    
    x(i,:) = b\A; % least squares regression
    
end

%% Convering output data to angles for viewing and deciding which data to use
theta = unwrap(atan2(x(:,2),x(:,1)));
figure(2)
plot(theta)
xlabel('Sample number');
ylabel('Attitude (rad)');
title('Attitude vs time -- Select when data is good');

%% getting only the good data (use Figure 1 to judge)
data_start = input('Input where to start analysis (sample #): ');
close gcf


% Discarding bad data that may have been accumulated initially
theta = theta(data_start:end);
t = t(data_start:end);
t = t - t(1);
r_1x = r_1x(data_start:end);
r_1y = r_1y(data_start:end);
r_2x = r_2x(data_start:end);
r_2y = r_2y(data_start:end);
r_3x = r_3x(data_start:end);
r_3y = r_3y(data_start:end);
r_4x = r_4x(data_start:end);
r_4y = r_4y(data_start:end);

%% Calculating derivatives of all variables
v_1x = diff(r_1x)/(t(2)-t(1));
v_1y = diff(r_1y)/(t(2)-t(1));
v_2x = diff(r_2x)/(t(2)-t(1));
v_2y = diff(r_2y)/(t(2)-t(1));
v_3x = diff(r_3x)/(t(2)-t(1));
v_3y = diff(r_3y)/(t(2)-t(1));
v_4x = diff(r_4x)/(t(2)-t(1));
v_4y = diff(r_4y)/(t(2)-t(1));
omega = diff(theta)/(t(2)-t(1));

a_1x = diff(v_1x)/(t(2)-t(1));
a_1y = diff(v_1y)/(t(2)-t(1));
a_2x = diff(v_2x)/(t(2)-t(1));
a_2y = diff(v_2y)/(t(2)-t(1));
a_3x = diff(v_3x)/(t(2)-t(1));
a_3y = diff(v_3y)/(t(2)-t(1));
a_4x = diff(v_4x)/(t(2)-t(1));
a_4y = diff(v_4y)/(t(2)-t(1));
alpha = diff(omega)/(t(2)-t(1));

%% Plotting velocity of raw data
figure(2)
plot(v_1x,v_1y,v_2x,v_2y,v_3x,v_3y,v_4x,v_4y)
legend('V1','V2','V3','V4')
axis equal
xlabel('Velocity in X [mm/s]');
ylabel('Velocity in Y [mm/s]');
title('Raw Marker velocity');

%% Plotting accleration of raw data
figure(6)
plot(a_1x,a_1y,a_2x,a_2y,a_3x,a_3y,a_4x,a_4y)
legend('LED1','LED2','LED3','LED4')
axis equal
xlabel('Acceleration in X [mm/s]');
ylabel('Acceleration in Y [mm/s]');
title('Raw Marker Acceleration');

%% Calculating centre of mass acceleration
circ1 = ([a_1x',a_1y',ones(size(a_1x'))]\[a_1x'.^2+a_1y'.^2])/2; % calculates centre of circle and outputs [xc yc U]
circ2 = ([a_2x',a_2y',ones(size(a_2x'))]\[a_2x'.^2+a_2y'.^2])/2; % calculates centre of circle and outputs [xc yc U]
circ3 = ([a_3x',a_3y',ones(size(a_3x'))]\[a_3x'.^2+a_3y'.^2])/2; % calculates centre of circle and outputs [xc yc U]
circ4 = ([a_4x',a_4y',ones(size(a_4x'))]\[a_4x'.^2+a_4y'.^2])/2; % calculates centre of circle and outputs [xc yc U]
xaccel = mean([circ1(1), circ2(1), circ3(1), circ4(1)]); % averaging 4 guesses at centre velocity
yaccel = mean([circ1(2), circ2(2), circ3(2), circ4(2)]);
fprintf(['Using regression, platform centre of gravity constant acceleration is [' num2str(xaccel) '; ' num2str(yaccel) '] mm/s\n']);

%% Removing acceleration from data
accel_drift_x = 0.5*xaccel*t.^2; % position that the centre of mass would have moved over the time span at the constant velocity
accel_drift_y = 0.5*yaccel*t.^2;

r_1x = r_1x - accel_drift_x;
r_1y = r_1y - accel_drift_y;
r_2x = r_2x - accel_drift_x;
r_2y = r_2y - accel_drift_y;
r_3x = r_3x - accel_drift_x;
r_3y = r_3y - accel_drift_y;
r_4x = r_4x - accel_drift_x;
r_4y = r_4y - accel_drift_y;

% Re-Calculating derivatives of all variables
v_1x = diff(r_1x)/(t(2)-t(1));
v_1y = diff(r_1y)/(t(2)-t(1));
v_2x = diff(r_2x)/(t(2)-t(1));
v_2y = diff(r_2y)/(t(2)-t(1));
v_3x = diff(r_3x)/(t(2)-t(1));
v_3y = diff(r_3y)/(t(2)-t(1));
v_4x = diff(r_4x)/(t(2)-t(1));
v_4y = diff(r_4y)/(t(2)-t(1));
omega = diff(theta)/(t(2)-t(1));

a_1x = diff(v_1x)/(t(2)-t(1));
a_1y = diff(v_1y)/(t(2)-t(1));
a_2x = diff(v_2x)/(t(2)-t(1));
a_2y = diff(v_2y)/(t(2)-t(1));
a_3x = diff(v_3x)/(t(2)-t(1));
a_3y = diff(v_3y)/(t(2)-t(1));
a_4x = diff(v_4x)/(t(2)-t(1));
a_4y = diff(v_4y)/(t(2)-t(1));
alpha = diff(omega)/(t(2)-t(1));

figure(9)
plot(a_1x,a_1y,a_2x,a_2y,a_3x,a_3y,a_4x,a_4y)
axis equal
legend('LED1','LED2','LED3','LED4')
title('Marker acceleration adjusted for acceleration');
xlabel('Table X [mm/s^2]');
ylabel('Table Y [mm/s^2]');

figure(8)
plot(v_1x,v_1y,v_2x,v_2y,v_3x,v_3y,v_4x,v_4y)
axis equal
legend('LED1','LED2','LED3','LED4')
title('Marker velocity adjusted for acceleration');
xlabel('Table X [mm/s]');
ylabel('Table Y [mm/s]');

figure(11)
plot(r_1x,r_1y,r_2x,r_2y,r_3x,r_3y,r_4x,r_4y)
axis equal
legend('LED1','LED2','LED3','LED4')
title('Marker position adjusted for acceleration');
xlabel('Table X [mm]');
ylabel('Table Y [mm]');


%% Calculating centre of mass velocity
circ1 = ([v_1x',v_1y',ones(size(v_1x'))]\[v_1x'.^2+v_1y'.^2])/2; % calculates centre of circle and outputs [xc yc U]
circ2 = ([v_2x',v_2y',ones(size(v_2x'))]\[v_2x'.^2+v_2y'.^2])/2; % calculates centre of circle and outputs [xc yc U]
circ3 = ([v_3x',v_3y',ones(size(v_3x'))]\[v_3x'.^2+v_3y'.^2])/2; % calculates centre of circle and outputs [xc yc U]
circ4 = ([v_4x',v_4y',ones(size(v_4x'))]\[v_4x'.^2+v_4y'.^2])/2; % calculates centre of circle and outputs [xc yc U]
xvel = mean([circ1(1), circ2(1), circ3(1), circ4(1)]); % averaging 4 guesses at centre velocity
yvel = mean([circ1(2), circ2(2), circ3(2), circ4(2)]);
fprintf(['Using circular regression, platform velocity is [' num2str(xvel) '; ' num2str(yvel) '] mm/s\n']);

%% Removing velocity from data
vel_drift_x = linspace(0,xvel*t(end),length(r_1x)); % position that the centre of mass would have moved over the time span at the constant velocity
vel_drift_y = linspace(0,yvel*t(end),length(r_1y));

r_1x = r_1x - vel_drift_x;
r_1y = r_1y - vel_drift_y;
r_2x = r_2x - vel_drift_x;
r_2y = r_2y - vel_drift_y;
r_3x = r_3x - vel_drift_x;
r_3y = r_3y - vel_drift_y;
r_4x = r_4x - vel_drift_x;
r_4y = r_4y - vel_drift_y;

% Re-Calculating derivatives
v_1x = diff(r_1x)/(t(2)-t(1));
v_1y = diff(r_1y)/(t(2)-t(1));
v_2x = diff(r_2x)/(t(2)-t(1));
v_2y = diff(r_2y)/(t(2)-t(1));
v_3x = diff(r_3x)/(t(2)-t(1));
v_3y = diff(r_3y)/(t(2)-t(1));
v_4x = diff(r_4x)/(t(2)-t(1));
v_4y = diff(r_4y)/(t(2)-t(1));

figure(10)
plot(v_1x,v_1y,v_2x,v_2y,v_3x,v_3y,v_4x,v_4y)
axis equal
legend('LED1','LED2','LED3','LED4')
title('Marker velocity adjusted for velocity');
xlabel('Table X [mm/s]');
ylabel('Table Y [mm/s]');

figure(3)
plot(r_1x,r_1y,r_2x,r_2y,r_3x,r_3y,r_4x,r_4y)
axis equal
legend('LED1','LED2','LED3','LED4')
title('Marker position adjusted for velocity');
xlabel('Table X [mm]');
ylabel('Table Y [mm]');


%% Finding centre of mass
% Since data points are rotating about the centre of mass, it is evident by
% the centre of the circle each marker traces out
circ1 = ([r_1x',r_1y',ones(size(r_1x'))]\[r_1x'.^2+r_1y'.^2])/2; % calculates centre of circle and outputs [xc yc U]
circ2 = ([r_2x',r_2y',ones(size(r_2x'))]\[r_2x'.^2+r_2y'.^2])/2; % calculates centre of circle and outputs [xc yc U]
circ3 = ([r_3x',r_3y',ones(size(r_3x'))]\[r_3x'.^2+r_3y'.^2])/2; % calculates centre of circle and outputs [xc yc U]
circ4 = ([r_4x',r_4y',ones(size(r_4x'))]\[r_4x'.^2+r_4y'.^2])/2; % calculates centre of circle and outputs [xc yc U]
r_comx = mean([circ1(1), circ2(1), circ3(1), circ4(1)]);
r_comy = mean([circ1(2), circ2(2), circ3(2), circ4(2)]);
fprintf(['Using circular regression, initial centre of gravity is at [' num2str(r_comx) '; ' num2str(r_comy) '] mm\n']);


%% Subtracting centre of mass position from the marker positions (i.e., map the centre of mass to the table origine)
r_1x = r_1x - r_comx;
r_1y = r_1y - r_comy;
r_2x = r_2x - r_comx;
r_2y = r_2y - r_comy;
r_3x = r_3x - r_comx;
r_3y = r_3y - r_comy;
r_4x = r_4x - r_comx;
r_4y = r_4y - r_comy;

figure(4)
plot(r_1x,r_1y,r_2x,r_2y,r_3x,r_3y,r_4x,r_4y)
axis equal
legend('LED1','LED2','LED3','LED4')
title('Marker Positions Adjusted to Origine of Table');
xlabel('Table X [mm]');
ylabel('Table Y [mm]');

%% Rotating marker positions into the body frame (should become a line)
for i = 1:length(r_1x)
    
    ROT = [cos(theta(i)), -sin(theta(i));
        sin(theta(i)),  cos(theta(i))]'; % C_bI = C_Ib'
    
    r_1b(i,:) = ROT*[r_1x(i); r_1y(i)];
    r_2b(i,:) = ROT*[r_2x(i); r_2y(i)];
    r_3b(i,:) = ROT*[r_3x(i); r_3y(i)];
    r_4b(i,:) = ROT*[r_4x(i); r_4y(i)];
    
end

figure(5)
hold on
%scatter([r_1b(1) r_2b(1) r_3b(1) r_4b(1)],r_2b,r_3b,r_4b)
scatter(r_1b(:,1),r_1b(:,2),45,'b','filled')
scatter(r_2b(:,1),r_2b(:,2),45,'g','filled')
scatter(r_3b(:,1),r_3b(:,2),45,'r','filled')
scatter(r_4b(:,1),r_4b(:,2),45,'c','filled')
scatter(0,0,100,'k','filled');
axis equal
legend('LED1','LED2','LED3','LED4');
xlabel('Body X [mm]');
ylabel('Body Y [mm]');
title('Marker positions wrt Centre of mass in Body Frame');

%% Finally averaging Ri/com in the body frame to get best guess at marker positions
R1COM = [mean(r_1b(:,1)); mean(r_1b(:,2))];
R2COM = [mean(r_2b(:,1)); mean(r_2b(:,2))];
R3COM = [mean(r_3b(:,1)); mean(r_3b(:,2))];
R4COM = [mean(r_4b(:,1)); mean(r_4b(:,2))];

fprintf(['Best guess at marker 1 is [' num2str(R1COM(1)) '; ' num2str(R1COM(2)) ']\n']);
fprintf(['Best guess at marker 2 is [' num2str(R2COM(1)) '; ' num2str(R2COM(2)) ']\n']);
fprintf(['Best guess at marker 3 is [' num2str(R3COM(1)) '; ' num2str(R3COM(2)) ']\n']);
fprintf(['Best guess at marker 4 is [' num2str(R4COM(1)) '; ' num2str(R4COM(2)) ']\n']);
