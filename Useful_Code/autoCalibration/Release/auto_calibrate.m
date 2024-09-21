% Finds the centre of mass of the platform. Collect data using
% auto_calibrate.exe

clear
clc
close all

global r1x r1y r2x r2y r3x r3y r4x r4y v1x v1y v2x v2y v3x v3y v4x v4y Theta Omega deltaT lastrcomx1 lastrcomy1 lastrcomx2 lastrcomy2 lastrcomx3 lastrcomy3 lastrcomx4 lastrcomy4
%% actual data
% r_21 = [289;3];
% r_32 = [0;-248];
% r_43 = [-288;0];
% r_14 = [-1;245];
% r_24 = [288;248];
% r_31 = [289;-245];
% 
% % Calculates the MOI of SPOT using ground truth data
% Fs = 20; % [Hz] sample frequency
% %filename = 'good_spin_data.txt'; % data file in same directory as this .m file
% filename = 'auto_calibrate_noarm_1.txt';
% columns = 13; % how many columns are in the text file
% fileID = fopen(filename); % opening the file
% 
% data = fscanf(fileID,'%f',[columns,Inf])'; % getting the data
% 
% t = data(:,1);
% r_1x = data(:,8); % top left
% r_1y = data(:,9);
% r_2x = data(:,11);% top right
% r_2y = data(:,12);
% r_3x = data(:,2); % bottom right
% r_3y = data(:,3);
% r_4x = data(:,5); % bottom left
% r_4y = data(:,6);
% 
% timesteps = t;

%% Simulated Data
r_21 = [300;0];
r_32 = [0;-300];
r_43 = [-300;0];
r_14 = [0;300];
r_24 = [300;300];
r_31 = [300;-300];

rate = 0.1; % [rad/s]
offset = [000;000]';
velocity = [10; 0]; % [mm/s] in Ix and Iy

r_1com = [-150;150];
r_2com = [150;150];
r_3com = [150;-150];
r_4com = [-150;-150];

r1angle = atan2(r_1com(2),r_1com(1));
r2angle = atan2(r_2com(2),r_2com(1));
r3angle = atan2(r_3com(2),r_3com(1));
r4angle = atan2(r_4com(2),r_4com(1));

timesteps = 0:1/10:100;
t = timesteps;
angles = timesteps*rate;

r1 = [cos(angles+r1angle)*norm(r_1com); sin(angles+r1angle)*norm(r_1com)] + [linspace(offset(1),offset(1)+timesteps(end)*velocity(1),length(t)); linspace(offset(2),offset(2)+timesteps(end)*velocity(2),length(t))];
r2 = [cos(angles+r2angle)*norm(r_2com); sin(angles+r2angle)*norm(r_2com)] + [linspace(offset(1),offset(1)+timesteps(end)*velocity(1),length(t)); linspace(offset(2),offset(2)+timesteps(end)*velocity(2),length(t))];
r3 = [cos(angles+r3angle)*norm(r_3com); sin(angles+r3angle)*norm(r_3com)] + [linspace(offset(1),offset(1)+timesteps(end)*velocity(1),length(t)); linspace(offset(2),offset(2)+timesteps(end)*velocity(2),length(t))];
r4 = [cos(angles+r4angle)*norm(r_4com); sin(angles+r4angle)*norm(r_4com)] + [linspace(offset(1),offset(1)+timesteps(end)*velocity(1),length(t)); linspace(offset(2),offset(2)+timesteps(end)*velocity(2),length(t))];
%keyboard
%% Attempting to rotate results back into body frame??
% 
% r1_subPos = r1 - [linspace(offset(1),offset(1)+timesteps(end)*velocity(1),length(t)); linspace(offset(2),offset(2)+timesteps(end)*velocity(2),length(t))];
% r2_subPos = r2 - [linspace(offset(1),offset(1)+timesteps(end)*velocity(1),length(t)); linspace(offset(2),offset(2)+timesteps(end)*velocity(2),length(t))];
% r3_subPos = r3 - [linspace(offset(1),offset(1)+timesteps(end)*velocity(1),length(t)); linspace(offset(2),offset(2)+timesteps(end)*velocity(2),length(t))];
% r4_subPos = r4 - [linspace(offset(1),offset(1)+timesteps(end)*velocity(1),length(t)); linspace(offset(2),offset(2)+timesteps(end)*velocity(2),length(t))];
% 
% 
% for i = 1:length(t)
%     
% 
%     ROT = [cos(angles(i)), -sin(angles(i));
%            sin(angles(i)),  cos(angles(i))]'; % C_bI = C_Ib'
% 
%     r1b(:,i) = ROT*r1_subPos(:,i);
%     r2b(:,i) = ROT*r2_subPos(:,i);
%     r3b(:,i) = ROT*r3_subPos(:,i);
%     r4b(:,i) = ROT*r4_subPos(:,i);
% 
% end
% 
% 
% plot(r1b(1,:),r1b(2,:),r2b(1,:),r2b(2,:),r3b(1,:),r3b(2,:),r4b(1,:),r4b(2,:))
% axis equal
% legend('R1','R2','R3','R4')
% return

r_1x = r1(1,:);
r_1y = r1(2,:);
r_2x = r2(1,:);
r_2y = r2(2,:);
r_3x = r3(1,:);
r_3y = r3(2,:);
r_4x = r4(1,:);
r_4y = r4(2,:);

% %plot(timesteps,r_4x,timesteps,r_4y,'r')
%

%%

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
    
    x(i,:) = b\A;
    
end

%% Convering x data to angles for viewing
theta = unwrap(atan2(x(:,2),x(:,1)));
figure(1)
plot(theta)
xlabel('Sample number');
ylabel('Attitude (rad)');

%% getting only the good data (use Figure 1 to judge
data_start = input('Input where to start analysis (sample #): ');

theta = theta(data_start:end);
t = t(data_start:end-1);
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

%% For each time step calculate COM

X = zeros(length(t),12);
deltaT = t(5)-t(4);
%X(1,11) = offset(1);
%X(1,12) = offset(2);
% X(1,11) = 1500;
% X(1,12) = 1500;

iterator = 5;

% lastrcomx1 = offset(1);
% lastrcomy1 = offset(2);
% lastrcomx2 = offset(1);
% lastrcomy2 = offset(2);
% lastrcomx3 = offset(1);
% lastrcomy3 = offset(2);
% lastrcomx4 = offset(1);
% lastrcomy4 = offset(2);

% lastrcomx1 = 0;
% lastrcomy1 = 0;
% lastrcomx2 = 0;
% lastrcomy2 = 0;
% lastrcomx3 = 0;
% lastrcomy3 = 0;
% lastrcomx4 = 0;
% lastrcomy4 = 0;

%x0 = [r_1com-5; r_2com; r_3com; r_4com];

for i = 1:length(t)
    
        A = [1, 0, -sin(theta(i))*omega(i), -cos(theta(i))*omega(i), 0, 0, 0, 0, 0, 0, 0, 0;
            0, 1,  cos(theta(i))*omega(i), -sin(theta(i))*omega(i), 0, 0, 0, 0, 0, 0, 0, 0;
            1, 0, 0, 0, -sin(theta(i))*omega(i), -cos(theta(i))*omega(i), 0, 0, 0, 0, 0, 0;
            0, 1, 0, 0,  cos(theta(i))*omega(i), -sin(theta(i))*omega(i), 0, 0, 0, 0, 0, 0;
            1, 0, 0, 0, 0, 0, -sin(theta(i))*omega(i), -cos(theta(i))*omega(i), 0, 0, 0, 0;
            0, 1, 0, 0, 0, 0,  cos(theta(i))*omega(i), -sin(theta(i))*omega(i), 0, 0, 0, 0;
            1, 0, 0, 0, 0, 0, 0, 0, -sin(theta(i))*omega(i), -cos(theta(i))*omega(i), 0, 0;
            0, 1, 0, 0, 0, 0, 0, 0,  cos(theta(i))*omega(i), -sin(theta(i))*omega(i), 0, 0;
            0, 0, cos(theta(i)), -sin(theta(i)), 0, 0, 0, 0, 0, 0, 1, 0;
            0, 0, sin(theta(i)),  cos(theta(i)), 0, 0, 0, 0, 0, 0, 0, 1;
            0, 0, 0, 0, cos(theta(i)), -sin(theta(i)), 0, 0, 0, 0, 1, 0;
            0, 0, 0, 0, sin(theta(i)),  cos(theta(i)), 0, 0, 0, 0, 0, 1;
            0, 0, 0, 0, 0, 0, cos(theta(i)), -sin(theta(i)), 0, 0, 1, 0;
            0, 0, 0, 0, 0, 0, sin(theta(i)),  cos(theta(i)), 0, 0, 0, 1;
            0, 0, 0, 0, 0, 0, 0, 0, cos(theta(i)), -sin(theta(i)), 1, 0;
            0, 0, 0, 0, 0, 0, 0, 0, sin(theta(i)),  cos(theta(i)), 0, 1];
        
%         A = [sin(theta(i))*omega(i), -cos(theta(i))*omega(i), 0, 0, 0, 0, 0, 0;
%             cos(theta(i))*omega(i), sin(theta(i))*omega(i), 0, 0, 0, 0, 0, 0;
%             0, 0, sin(theta(i))*omega(i), -cos(theta(i))*omega(i), 0, 0, 0, 0;
%             0, 0,  cos(theta(i))*omega(i), sin(theta(i))*omega(i), 0, 0, 0, 0;
%             0, 0, 0, 0, sin(theta(i))*omega(i), -cos(theta(i))*omega(i), 0, 0;
%             0, 0, 0, 0,  cos(theta(i))*omega(i), sin(theta(i))*omega(i), 0, 0;
%             0, 0, 0, 0, 0, 0, sin(theta(i))*omega(i), -cos(theta(i))*omega(i);
%             0, 0, 0, 0, 0, 0,  cos(theta(i))*omega(i), sin(theta(i))*omega(i);
%             cos(theta(i)), -sin(theta(i)), 0, 0, 0, 0, 0, 0;
%             sin(theta(i)),  cos(theta(i)), 0, 0, 0, 0, 0, 0;
%             0, 0, cos(theta(i)), -sin(theta(i)), 0, 0, 0, 0;
%             0, 0, sin(theta(i)),  cos(theta(i)), 0, 0, 0, 0;
%             0, 0, 0, 0, cos(theta(i)), -sin(theta(i)), 0, 0;
%             0, 0, 0, 0, sin(theta(i)),  cos(theta(i)), 0, 0;
%             0, 0, 0, 0, 0, 0, cos(theta(i)), -sin(theta(i));
%             0, 0, 0, 0, 0, 0, sin(theta(i)),  cos(theta(i))];
        
 
%         
%     A = [1, 0, -sin(-theta(i))*omega(i), -cos(-theta(i))*omega(i), 0, 0, 0, 0, 0, 0, 0, 0;
%             0, 1,  cos(-theta(i))*omega(i), -sin(-theta(i))*omega(i), 0, 0, 0, 0, 0, 0, 0, 0;
%             1, 0, 0, 0, -sin(-theta(i))*omega(i), -cos(-theta(i))*omega(i), 0, 0, 0, 0, 0, 0;
%             0, 1, 0, 0,  cos(-theta(i))*omega(i), -sin(-theta(i))*omega(i), 0, 0, 0, 0, 0, 0;
%             1, 0, 0, 0, 0, 0, -sin(-theta(i))*omega(i), -cos(-theta(i))*omega(i), 0, 0, 0, 0;
%             0, 1, 0, 0, 0, 0,  cos(-theta(i))*omega(i), -sin(-theta(i))*omega(i), 0, 0, 0, 0;
%             1, 0, 0, 0, 0, 0, 0, 0, -sin(-theta(i))*omega(i), -cos(-theta(i))*omega(i), 0, 0;
%             0, 1, 0, 0, 0, 0, 0, 0,  cos(-theta(i))*omega(i), -sin(-theta(i))*omega(i), 0, 0;
%             0, 0, cos(-theta(i)), -sin(-theta(i)), 0, 0, 0, 0, 0, 0, 1, 0;
%             0, 0, sin(-theta(i)),  cos(-theta(i)), 0, 0, 0, 0, 0, 0, 0, 1;
%             0, 0, 0, 0, cos(-theta(i)), -sin(-theta(i)), 0, 0, 0, 0, 1, 0;
%             0, 0, 0, 0, sin(-theta(i)),  cos(-theta(i)), 0, 0, 0, 0, 0, 1;
%             0, 0, 0, 0, 0, 0, cos(-theta(i)), -sin(-theta(i)), 0, 0, 1, 0;
%             0, 0, 0, 0, 0, 0, sin(-theta(i)),  cos(-theta(i)), 0, 0, 0, 1;
%             0, 0, 0, 0, 0, 0, 0, 0, cos(-theta(i)), -sin(-theta(i)), 1, 0;
%             0, 0, 0, 0, 0, 0, 0, 0, sin(-theta(i)),  cos(-theta(i)), 0, 1];
    
    
    %      A = [1, 0, omega(i), -omega(i), 0, 0, 0, 0, 0, 0, 0, 0;
    %          0, 1,  omega(i), -omega(i), 0, 0, 0, 0, 0, 0, 0, 0;
    %          1, 0, 0, 0, omega(i), -omega(i), 0, 0, 0, 0, 0, 0;
    %          0, 1, 0, 0, omega(i), -omega(i), 0, 0, 0, 0, 0, 0;
    %          1, 0, 0, 0, 0, 0, omega(i), -omega(i), 0, 0, 0, 0;
    %          0, 1, 0, 0, 0, 0, omega(i), -omega(i), 0, 0, 0, 0;
    %          1, 0, 0, 0, 0, 0, 0, 0, omega(i), -omega(i), 0, 0;
    %          0, 1, 0, 0, 0, 0, 0, 0, omega(i), -omega(i), 0, 0;
    %          0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0;
    %          0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1;
    %          0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0;
    %          0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1;
    %          0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0;
    %          0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1;
    %          0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0;
    %          0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1];
    
        b = [v_1x(i);
            v_1y(i);
            v_2x(i);
            v_2y(i);
            v_3x(i);
            v_3y(i);
            v_4x(i);
            v_4y(i);
            r_1x(i);
            r_1y(i);
            r_2x(i);
            r_2y(i);
            r_3x(i);
            r_3y(i);
            r_4x(i);
            r_4y(i)];
    %
    
%            A = [1, 0, -sin(theta(i))*omega(i), -cos(theta(i))*omega(i), 0, 0, 0, 0, 0, 0;
%              0, 1,  cos(theta(i))*omega(i), -sin(theta(i))*omega(i), 0, 0, 0, 0, 0, 0;
%              1, 0, 0, 0, -sin(theta(i))*omega(i), -cos(theta(i))*omega(i), 0, 0, 0, 0;
%              0, 1, 0, 0,  cos(theta(i))*omega(i), -sin(theta(i))*omega(i), 0, 0, 0, 0;
%              1, 0, 0, 0, 0, 0, -sin(theta(i))*omega(i), -cos(theta(i))*omega(i), 0, 0;
%              0, 1, 0, 0, 0, 0,  cos(theta(i))*omega(i), -sin(theta(i))*omega(i), 0, 0;
%              1, 0, 0, 0, 0, 0, 0, 0, -sin(theta(i))*omega(i), -cos(theta(i))*omega(i);
%              0, 1, 0, 0, 0, 0, 0, 0,  cos(theta(i))*omega(i), -sin(theta(i))*omega(i);
%              0, 0, cos(theta(i)), -sin(theta(i)), 0, 0, 0, 0, 0, 0;
%              0, 0, sin(theta(i)),  cos(theta(i)), 0, 0, 0, 0, 0, 0;
%              0, 0, 0, 0, cos(theta(i)), -sin(theta(i)), 0, 0, 0, 0;
%              0, 0, 0, 0, sin(theta(i)),  cos(theta(i)), 0, 0, 0, 0;
%              0, 0, 0, 0, 0, 0, cos(theta(i)), -sin(theta(i)), 0, 0;
%              0, 0, 0, 0, 0, 0, sin(theta(i)),  cos(theta(i)), 0, 0;
%              0, 0, 0, 0, 0, 0, 0, 0, cos(theta(i)), -sin(theta(i));
%              0, 0, 0, 0, 0, 0, 0, 0, sin(theta(i)),  cos(theta(i))];
    %
    %          X(i,11) = X(i-1,11) + X(i-1,1)*deltaT;
    %          X(i,12) = X(i-1,12) + X(i-1,2)*deltaT;
    %
    %       b = [v_1x(i);
    %           v_1y(i);
    %           v_2x(i);
    %           v_2y(i);
    %           v_3x(i);
    %           v_3y(i);
    %           v_4x(i);
    %           v_4y(i);
    %           r_1x(i)-X(i,11);
    %           r_1y(i)-X(i,12);
    %           r_2x(i)-X(i,11);
    %           r_2y(i)-X(i,12);
    %           r_3x(i)-X(i,11);
    %           r_3y(i)-X(i,12);
    %           r_4x(i)-X(i,11);
    %           r_4y(i)-X(i,12)];
    %
    %     for j = 1:iterator
    %
    %    A = [ -sin(theta(i))*omega(i), -cos(theta(i))*omega(i), 0, 0, 0, 0, 0, 0, 0, 0;
    %           cos(theta(i))*omega(i), -sin(theta(i))*omega(i), 0, 0, 0, 0, 0, 0, 0, 0;
    %          0, 0, -sin(theta(i))*omega(i), -cos(theta(i))*omega(i), 0, 0, 0, 0, 0, 0;
    %          0, 0,  cos(theta(i))*omega(i), -sin(theta(i))*omega(i), 0, 0, 0, 0, 0, 0;
    %          0, 0, 0, 0, -sin(theta(i))*omega(i), -cos(theta(i))*omega(i), 0, 0, 0, 0;
    %          0, 0, 0, 0,  cos(theta(i))*omega(i), -sin(theta(i))*omega(i), 0, 0, 0, 0;
    %          0, 0, 0, 0, 0, 0, -sin(theta(i))*omega(i), -cos(theta(i))*omega(i), 0, 0;
    %          0, 0, 0, 0, 0, 0,  cos(theta(i))*omega(i), -sin(theta(i))*omega(i), 0, 0;
    %          cos(theta(i)), -sin(theta(i)), 0, 0, 0, 0, 0, 0, 1, 0;
    %          sin(theta(i)),  cos(theta(i)), 0, 0, 0, 0, 0, 0, 0, 1;
    %          0, 0, cos(theta(i)), -sin(theta(i)), 0, 0, 0, 0, 1, 0;
    %          0, 0, sin(theta(i)),  cos(theta(i)), 0, 0, 0, 0, 0, 1;
    %          0, 0, 0, 0, cos(theta(i)), -sin(theta(i)), 0, 0, 1, 0;
    %          0, 0, 0, 0, sin(theta(i)),  cos(theta(i)), 0, 0, 0, 1;
    %          0, 0, 0, 0, 0, 0, cos(theta(i)), -sin(theta(i)), 1, 0;
    %          0, 0, 0, 0, 0, 0, sin(theta(i)),  cos(theta(i)), 0, 1];
    %
    %      if i == 6 || i == 7
    %          keyboard;
    %      end
    %         if i == 1 || i == 2 || i == 3 || i == 4 || i == 5
    %             velx = 0;
    %             vely = 0;
    %         else
    %             velx = (X(i-1,11) - X(i-2,11))/deltaT;
    %             vely = (X(i-1,12) - X(i-2,12))/deltaT;
    %         end
    %         b = [v_1x(i)-velx;
    %             v_1y(i)-vely;
    %             v_2x(i)-velx;
    %             v_2y(i)-vely;
    %             v_3x(i)-velx;
    %             v_3y(i)-vely;
    %             v_4x(i)-velx;
    %             v_4y(i)-vely;
    %             r_1x(i);
    %             r_1y(i);
    %             r_2x(i);
    %             r_2y(i);
    %             r_3x(i);
    %             r_3y(i);
    %             r_4x(i);
    %             r_4y(i)];
    %
    %
    %         %X(i,3:12) = A\b;
    %         X(i,3:12) = pinv(A)*b;
    %     end
    %
    % A = [-omega(i), 0, 1, 0;
    %       0, omega(i), 0, 1;
    %       -omega(i), 0, 1, 0;
    %       0, omega(i), 0, 1;
    %       -omega(i), 0, 1, 0;
    %       0, omega(i), 0, 1;
    %       -omega(i), 0, 1, 0;
    %       0, omega(i), 0, 1];
    %
    %   b = [v_1x(i)-omega(i)*r_1x(i);
    %        v_1y(i)+omega(i)*r_1y(i);
    %        v_2x(i)-omega(i)*r_2x(i);
    %        v_2y(i)+omega(i)*r_2y(i);
    %        v_3x(i)-omega(i)*r_3x(i);
    %        v_3y(i)+omega(i)*r_3y(i);
    %        v_4x(i)-omega(i)*r_4x(i);
    %        v_4y(i)+omega(i)*r_4y(i)];
    %
    %    Y(i,:) = pinv(A)*b;
    %
    %    X(i,3) = r_1x(i) - Y(i,1);
    %    X(i,4) = r_1y(i) - Y(i,2);
    %    X(i,5) = r_2x(i) - Y(i,1);
    %    X(i,6) = r_2y(i) - Y(i,2);
    %    X(i,7) = r_3x(i) - Y(i,1);
    %    X(i,8) = r_3y(i) - Y(i,2);
    %    X(i,9) = r_4x(i) - Y(i,1);
    %    X(i,10) = r_4y(i) - Y(i,2);
    
    %% Trying to minimize the difference between the derivative of position and the velocity
%     r1x = r_1x(i);
%     r1y = r_1y(i);
%     r2x = r_2x(i);
%     r2y = r_2y(i);
%     r3x = r_3x(i);
%     r3y = r_3y(i);
%     r4x = r_4x(i);
%     r4y = r_4y(i);
%     v1x = v_1x(i);
%     v1y = v_1y(i);
%     v2x = v_2x(i);
%     v2y = v_2y(i);
%     v3x = v_3x(i);
%     v3y = v_3y(i);
%     v4x = v_4x(i);
%     v4y = v_4y(i);
%     Theta = theta(i);
%     Omega = omega(i);
%     
%     x = fminsearch(@drdtMinusVel,x0);
%     
%     rcomx1 = r1x - x(1);
%     rcomy1 = r1y - x(2);
%     rcomx2 = r2x - x(3);
%     rcomy2 = r2y - x(4);
%     rcomx3 = r3x - x(5);
%     rcomy3 = r3y - x(6);
%     rcomx4 = r4x - x(7);
%     rcomy4 = r4y - x(8);
%     
%     rcomMean = mean([rcomx1 rcomy1 rcomx2 rcomy2 rcomx3 rcomy3 rcomx4 rcomy4]);
%     %keyboard
%     lastrcomx1 = rcomMean;
%     lastrcomy1 = rcomMean;
%     lastrcomx2 = rcomMean;
%     lastrcomy2 = rcomMean;
%     lastrcomx3 = rcomMean;
%     lastrcomy3 = rcomMean;
%     lastrcomx4 = rcomMean;
%     lastrcomy4 = rcomMean;
%    
%     %%
%     
%     Z(i,:) = [x(1), x(2), x(3), x(4), x(5), x(6), x(7), x(8)];
%     x0 = Z(i,:);
%X(i,3:10) = A\b;

X(i,:) = pinv(A)*b;

end
% close all
% plot(Z)
% 
% 
% fprintf(['Best guess at marker 1 is [' num2str(mean(Z(:,1))) ', ' num2str(mean(Z(:,2))) '] mm \n']);
% fprintf(['Best guess at marker 2 is [' num2str(mean(Z(:,3))) ', ' num2str(mean(Z(:,4))) '] mm\n']);
% fprintf(['Best guess at marker 3 is [' num2str(mean(Z(:,5))) ', ' num2str(mean(Z(:,6))) '] mm\n']);
% fprintf(['Best guess at marker 4 is [' num2str(mean(Z(:,7))) ', ' num2str(mean(Z(:,8))) '] mm\n']);
% 
% Z(end,:)
% return


% temp = X(2:end,:);
% clear X
% X = temp;
% t = t(2:end);
%% Attempting to rotate results back into body frame??
% for i = 1:length(t)
%
%     ROT = [cos(theta(i)), -sin(theta(i));
%            sin(theta(i)),  cos(theta(i))]'; % C_bI = C_Ib'
%
%     X(i,3:4) = (ROT*X(i,3:4)')';
%     X(i,5:6) = (ROT*X(i,5:6)')';
%     X(i,7:8) = (ROT*X(i,7:8)')';
%     X(i,9:10) = (ROT*X(i,9:10)')';
%     X(i,11:12) = (ROT*X(i,11:12)')';
% end

%% Plotting Results
figure(2)
plot(t,X(:,1),'r',t,X(:,2),'g')
legend('VcomX','VcomY');
title('COM Velocities');

figure(7)
plot(t,X(:,11),'r',t,X(:,12),'g')
legend('rcomx','rcomy')
title('COM Position');

figure(3)
plot(t,X(:,3),'r',t,X(:,4),'g')
legend('r1/comx','r1/comy')
title('R1');

figure(4)
plot(t,X(:,5),'r',t,X(:,6),'g')
legend('r2/comx','r2/comy')
title('R2');

figure(5)
plot(t,X(:,7),'r',t,X(:,8),'g')
legend('r3/comx','r3/comy')
title('R3');

figure(6)
plot(t,X(:,9),'r',t,X(:,10),'g')
legend('r4/comx','r4/comy')
title('R4');

figure(8)
plot(X(:,11),X(:,12))
xlabel('Inertial X');
ylabel('Inertial Y');
title('COM X vs Y position');
axis equal
%%
% figure(9)
% plot(X(:,3),X(:,4))
% legend('LED 1');
% axis equal
%
% figure(10)
% plot(X(:,5),X(:,6))
% legend('LED 2');
% axis equal
%
% figure(11)
% plot(X(:,7),X(:,8))
% legend('LED 3');
% axis equal
%
% figure(12)
% plot(X(:,9),X(:,10))
% legend('LED 4');
% axis equal
%
figure(13)
plot(X(:,3),X(:,4),X(:,5),X(:,6),X(:,7),X(:,8),X(:,9),X(:,10))
axis equal
legend('LED 1','LED 2','LED 3','LED 4');


%% Taking the mean as a guess
r1x = mean(X(:,3));
r1y = mean(X(:,4));
r2x = mean(X(:,5));
r2y = mean(X(:,6));
r3x = mean(X(:,7));
r3y = mean(X(:,8));
r4x = mean(X(:,9));
r4y = mean(X(:,10));

fprintf(['Average guess at marker 1 is [' num2str(r1x) ', ' num2str(r1y) ']\n']);
fprintf(['Average guess at marker 2 is [' num2str(r2x) ', ' num2str(r2y) ']\n']);
fprintf(['Average guess at marker 3 is [' num2str(r3x) ', ' num2str(r3y) ']\n']);
fprintf(['Average guess at marker 4 is [' num2str(r4x) ', ' num2str(r4y) ']\n']);

%% Attempting to use regression on the circles produced in Figure(13) to find their centres

circ = ([X(:,3),X(:,4),ones(size(X(:,3)))]\[X(:,3).^2+X(:,4).^2])/2; % calculates centre of circle and outputs [xc yc U]
R = sqrt((circ(1)^2+circ(2)^2)/4-circ(3)); % calculates radius of the circle (not needed but just incase)
fprintf(['Using circular regression, the postion of LED 1 is [' num2str(circ(1)) ', ' num2str(circ(2)) '] mm\n']);

circ = ([X(:,5),X(:,6),ones(size(X(:,3)))]\[X(:,5).^2+X(:,6).^2])/2; % calculates centre of circle and outputs [xc yc U]
R = sqrt((circ(1)^2+circ(2)^2)/4-circ(3)); % calculates radius of the circle (not needed but just incase)
fprintf(['Using circular regression, the postion of LED 2 is [' num2str(circ(1)) ', ' num2str(circ(2)) '] mm\n']);

circ = ([X(:,7),X(:,8),ones(size(X(:,3)))]\[X(:,7).^2+X(:,8).^2])/2; % calculates centre of circle and outputs [xc yc U]
R = sqrt((circ(1)^2+circ(2)^2)/4-circ(3)); % calculates radius of the circle (not needed but just incase)
fprintf(['Using circular regression, the postion of LED 3 is [' num2str(circ(1)) ', ' num2str(circ(2)) '] mm\n']);

circ = ([X(:,9),X(:,10),ones(size(X(:,3)))]\[X(:,9).^2+X(:,10).^2])/2; % calculates centre of circle and outputs [xc yc U]
R = sqrt((circ(1)^2+circ(2)^2)/4-circ(3)); % calculates radius of the circle (not needed but just incase)
fprintf(['Using circular regression, the postion of LED 4 is [' num2str(circ(1)) ', ' num2str(circ(2)) '] mm\n']);

