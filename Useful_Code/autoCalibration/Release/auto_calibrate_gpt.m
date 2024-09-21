% Finds the centre of mass of the platform. Collect data using
% auto_calibrate.exe

clear
clc
close all

global r1x r1y r2x r2y r3x r3y r4x r4y v1x v1y v2x v2y v3x v3y v4x v4y Theta Omega deltaT lastrcomx1 lastrcomy1 lastrcomx2 lastrcomy2 lastrcomx3 lastrcomy3 lastrcomx4 lastrcomy4
%% actual data
r_21 = [289;3];
r_32 = [0;-248];
r_43 = [-288;0];
r_14 = [-1;245];
r_24 = [288;248];
r_31 = [289;-245];

% Calculates the MOI of SPOT using ground truth data
Fs = 20; % [Hz] sample frequency
%filename = 'good_spin_data.txt'; % data file in same directory as this .m file
filename = 'auto_calibrate_servicer2.txt';
columns = 13; % how many columns are in the text file
fileID = fopen(filename); % opening the file

data = fscanf(fileID,'%f',[columns,Inf])'; % getting the data

t = data(:,1);
x1 = data(:,8); % top left
y1 = data(:,9);
x2 = data(:,11);% top right
y2 = data(:,12);
x3 = data(:,2); % bottom right
y3 = data(:,3);
x4 = data(:,5); % bottom left
y4 = data(:,6);

% Remove translational drift
mean_x = (x1 + x2 + x3 + x4) / 4;
mean_y = (y1 + y2 + y3 + y4) / 4;

x1_centered = x1 - mean_x;
y1_centered = y1 - mean_y;
x2_centered = x2 - mean_x;
y2_centered = y2 - mean_y;
x3_centered = x3 - mean_x;
y3_centered = y3 - mean_y;
x4_centered = x4 - mean_x;
y4_centered = y4 - mean_y;

% Example data points with drift and offset
x = [x1_centered, x2_centered, x3_centered, x4_centered];  % Replace with actual data
y = [y1_centered, y2_centered, y3_centered, y4_centered];  % Replace with actual data

% Step 1: Remove Drift
[x_detrended, y_detrended] = remove_drift(x, y);

% Step 2: Move to Center of Geometry
[x_centered, y_centered, x_shift, y_shift] = move_to_center_of_geometry(x_detrended, y_detrended);

% Step 3: Fit Circle and Calculate Correction
[a, b, r, x_correction, y_correction] = fit_circle_and_correct(x_centered, y_centered);

% Display results
fprintf('Center: (%.2f, %.2f), Radius: %.2f\n', a, b, r);
fprintf('X Correction: %.2f, Y Correction: %.2f\n', x_correction, y_correction);



% % Remove translational drift
% mean_x = (x1 + x2 + x3 + x4) / 4;
% mean_y = (y1 + y2 + y3 + y4) / 4;
% 
% x1_centered = x1 - mean_x;
% y1_centered = y1 - mean_y;
% x2_centered = x2 - mean_x;
% y2_centered = y2 - mean_y;
% x3_centered = x3 - mean_x;
% y3_centered = y3 - mean_y;
% x4_centered = x4 - mean_x;
% y4_centered = y4 - mean_y;
% 
% function [a, b, r] = fit_circle(x, y)
%     % Ensure the data is in column vectors
%     x = x(:);
%     y = y(:);
% 
%     % Formulate the linear system
%     A = [2*x, 2*y, ones(length(x), 1)];
%     b = x.^2 + y.^2;
% 
%     % Solve the linear system using least squares
%     theta = A \ b;
% 
%     % Extract the circle parameters
%     a = theta(1);
%     b = theta(2);
%     c = theta(3);
%     r = sqrt(a^2 + b^2 + c);
% end
% 
% 
% % Fit the circle
% [a, b, r] = fit_circle(x1_centered, y1_centered);
% fprintf('Center: (%.2f, %.2f), Radius: %.2f\n', a, b, r);
% 
% 
% % Fit the circle
% [a, b, r] = fit_circle(x2_centered, y2_centered);
% fprintf('Center: (%.2f, %.2f), Radius: %.2f\n', a, b, r);
% 
% % Fit the circle
% [a, b, r] = fit_circle(x3_centered, y3_centered);
% fprintf('Center: (%.2f, %.2f), Radius: %.2f\n', a, b, r);
% 
% % Fit the circle
% [a, b, r] = fit_circle(x4_centered, y4_centered);
% fprintf('Center: (%.2f, %.2f), Radius: %.2f\n', a, b, r);

% % Function to fit a circle
% function [xc, yc, R, a] = circfit(x, y)
%     x = x(:);
%     y = y(:);
%     A = [x y ones(size(x))];
%     b = -(x.^2 + y.^2);
%     a = A\b;
%     xc = -a(1)/2;
%     yc = -a(2)/2;
%     R = sqrt((a(1)^2 + a(2)^2)/4 - a(3));
% end
% 
% % Fit circular paths
% [xc1, yc1, ~, ~] = circfit(x1_centered, y1_centered);
% [xc2, yc2, ~, ~] = circfit(x2_centered, y2_centered);
% [xc3, yc3, ~, ~] = circfit(x3_centered, y3_centered);
% [xc4, yc4, ~, ~] = circfit(x4_centered, y4_centered);
% 
% % Find the common center
% xc_avg = mean([xc1, xc2, xc3, xc4]);
% yc_avg = mean([yc1, yc2, yc3, yc4]);
% 
% % The estimated center of gravity (CG)
% CG_x = xc_avg + mean_x;
% CG_y = yc_avg + mean_y;
% 
% fprintf('Estimated CG: (%.2f, %.2f)\n', CG_x, CG_y);
