close all
clc
% load("Saved Data\dataClass_rt")
dataClass = dataClass_rt;

%% Time
Time = dataClass.Time_s;


%% Red Data
States_Red = [dataClass.RED_Px_m, dataClass.RED_Py_m, dataClass.RED_Rz_rad];
Desired_States_Red = [dataClass.CustomUserData48, dataClass.CustomUserData49, dataClass.CustomUserData50];
DAC_Gains = [dataClass.CustomUserData51,dataClass.CustomUserData52,dataClass.CustomUserData53,dataClass.CustomUserData54,dataClass.CustomUserData55,dataClass.CustomUserData56];

%% Black Data
States_Blue = [dataClass.BLACK_Px_m,dataClass.BLACK_Py_m,dataClass.BLACK_Rz_rad];
TX2_Measurements_Black = [dataClass.CustomUserData57,dataClass.CustomUserData58, dataClass.CustomUserData59];
States_Black_Measured = [dataClass.CustomUserData63,dataClass.CustomUserData64,dataClass.CustomUserData65];
States_Black_Estimate = [dataClass.CustomUserData60,dataClass.CustomUserData61,dataClass.CustomUserData62];
Black_Data_Rejected = [dataClass.CustomUserData72];
Black_Detection = dataClass.CustomUserData75;

%% Blue Data
States_Black = [dataClass.BLUE_Px_m,dataClass.BLUE_Py_m,dataClass.BLUE_Rz_rad];
TX2_Measurements_Blue = [dataClass.CustomUserData66,dataClass.CustomUserData67];
States_Blue_Measured = [dataClass.CustomUserData70,dataClass.CustomUserData71];
States_Blue_Estimate = [dataClass.CustomUserData68,dataClass.CustomUserData69];
Blue_Data_Rejected = [dataClass.CustomUserData73];
BLUE_Detection = dataClass.CustomUserData74;

%% Find indices of changes in Black and Blue Detection
change_idx_black = find(diff(Black_Detection) ~= 0);
change_idx_blue = find(diff(BLUE_Detection) ~= 0);

%% Plots
figure % Actual Vs Desired
subplot(3,1,1)
plot(Time, States_Red(:,1), 'g', 'DisplayName', 'Actual X')
hold on
plot(Time, Desired_States_Red(:,1), 'r', 'DisplayName', 'Desired X','LineStyle','--')
scatter(Time(change_idx_black), States_Red(change_idx_black, 1), 'k', 'filled', 'DisplayName', 'Phase Change')
scatter(Time(change_idx_blue), States_Red(change_idx_blue, 1), 'b', 'filled', 'DisplayName', 'Blue Detection')
title('X Data: Actual vs Desired')
xlabel('Time (s)')
ylabel('Position (m)')
legend

subplot(3,1,2)
plot(Time, States_Red(:,2), 'r', 'DisplayName', 'Actual Y')
hold on
plot(Time, Desired_States_Red(:,2), 'g', 'DisplayName', 'Desired Y')
scatter(Time(change_idx_black), States_Red(change_idx_black, 2), 'k', 'filled', 'DisplayName', 'Phase Change')
scatter(Time(change_idx_blue), States_Red(change_idx_blue, 2), 'b', 'filled', 'DisplayName', 'Blue Detection')
title('Y Data: Actual vs Desired')
xlabel('Time (s)')
ylabel('Position (m)')
legend

subplot(3,1,3)
plot(Time, States_Red(:,3), 'r', 'DisplayName', 'Actual Theta')
hold on
plot(Time, Desired_States_Red(:,3), 'g', 'DisplayName', 'Desired Theta')
scatter(Time(change_idx_black), States_Red(change_idx_black, 3), 'k', 'filled', 'DisplayName', 'Phase Change')
scatter(Time(change_idx_blue), States_Red(change_idx_blue, 3), 'b', 'filled', 'DisplayName', 'Blue Detection')
title('Theta Data: Actual vs Desired')
xlabel('Time (s)')
ylabel('Angle (rad)')
legend

%% Black Data
figure 
subplot(3,1,1)
plot(Time, States_Black(:,1), 'k', 'DisplayName', 'Black States')
hold on
plot(Time, States_Black_Estimate(:,1), 'b', 'DisplayName', 'Black State Estimate')
plot(Time, States_Black_Measured(:,1), 'g*', 'DisplayName', 'Black Measured States')
rejected_indices_x_black = find(Black_Data_Rejected == 1);
plot(Time(rejected_indices_x_black), States_Black_Estimate(rejected_indices_x_black, 1), 'r', 'DisplayName', 'Rejected Data')
title('X Data: Blacks')
xlabel('Time (s)')
ylabel('Position (m)')
legend

subplot(3,1,2)
plot(Time, States_Black(:,2), 'k', 'DisplayName', 'Black States')
hold on
plot(Time, States_Black_Estimate(:,2), 'b', 'DisplayName', 'Black State Estimate')
plot(Time, States_Black_Measured(:,2), 'g*', 'DisplayName','Black Measured States')
rejected_indices_y_black = find(Black_Data_Rejected == 1);
plot(Time(rejected_indices_y_black), States_Black_Estimate(rejected_indices_y_black, 2), 'r', 'DisplayName', 'Rejected Data')
title('Y Data: Black States')
xlabel('Time (s)')
ylabel('Position (m)')
legend

subplot(3,1,3)
plot(Time, States_Black(:,3), 'k', 'DisplayName', 'Black States')
hold on
plot(Time, States_Black_Estimate(:,3), 'b', 'DisplayName', 'Black State Estimate')
plot(Time, States_Black_Measured(:,3), 'g*', 'DisplayName', 'Black Measured States')
rejected_indices_theta_black = find(Black_Data_Rejected == 1);
plot(Time(rejected_indices_theta_black), States_Black_Estimate(rejected_indices_theta_black, 3), 'r', 'DisplayName', 'Rejected Data')
title('Theta Data: Black States')
xlabel('Time (s)')
ylabel('Angle (rad)')
legend

%% Blue Data
figure
subplot(2,1,1)
plot(Time, States_Blue(:,1), 'k', 'DisplayName', 'Blue States')
hold on
plot(Time, States_Blue_Estimate(:,1), 'b', 'DisplayName', 'Blue State Estimate')
plot(Time, States_Blue_Measured(:,1), 'g*', 'DisplayName', 'Blue Measured States')
rejected_indices_x = find(Blue_Data_Rejected == 1);
plot(Time(rejected_indices_x), States_Blue_Estimate(rejected_indices_x, 1), 'r', 'DisplayName', 'Rejected Data')
title('X Data: Blues')
xlabel('Time (s)')
ylabel('Position (m)')
legend

subplot(2,1,2)
plot(Time, States_Blue(:,2), 'k', 'DisplayName', 'Blue States')
hold on
plot(Time, States_Blue_Estimate(:,2), 'b', 'DisplayName', 'Blue State Estimate')
plot(Time, States_Blue_Measured(:,2), 'g*', 'DisplayName', 'Blue Measured States')
rejected_indices_y = find(Blue_Data_Rejected == 1);
plot(Time(rejected_indices_y), States_Blue_Estimate(rejected_indices_y, 2), 'r', 'DisplayName', 'Rejected Data')
title('Y Data: Blue States')
xlabel('Time (s)')
ylabel('Position (m)')
legend

%% Plot DAC Gains
figure
subplot(3,1,1)
plot(Time, DAC_Gains(:,1), 'b', 'DisplayName', 'kp X')
hold on
plot(Time, DAC_Gains(:,2), 'r', 'DisplayName', 'kd X')
title('DAC Gains for X')
xlabel('Time (s)')
ylabel('Gains')
legend

subplot(3,1,2)
plot(Time, DAC_Gains(:,3), 'b', 'DisplayName', 'kp Y')
hold on
plot(Time, DAC_Gains(:,4), 'r', 'DisplayName', 'kd Y')
title('DAC Gains for Y')
xlabel('Time (s)')
ylabel('Gains')
legend

subplot(3,1,3)
plot(Time, DAC_Gains(:,5), 'b', 'DisplayName', 'kp Theta')
hold on
plot(Time, DAC_Gains(:,6), 'r', 'DisplayName', 'kd Theta')
title('DAC Gains for Theta')
xlabel('Time (s)')
ylabel('Gains')
legend