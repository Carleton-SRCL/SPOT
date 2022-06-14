% Estimate CG using 3 scales.

% User must enter mass measurements, LED positions, and thruster offsets

clear;
clc;
close('all')

% Black with panels and an air tank * USER ENTERED DATA *
Ab = 5463/1000; % [kg] Left-middle-edge
Bb = 3624/1000; % [kg] Right-back corner
Cb = 2180/1000; % [kg] Right-front corner

Mblue = Ab+Bb+Cb;

fprintf('The total mass for blue is: %.4f kg.\n',Mblue);

A_to_lineBC_distance = 0.3;
BC_distance = 0.1778;

Xblue = ((Cb-Bb)*BC_distance)/(2*Mblue);
Yblue = 0.15-((((Bb+Cb)*A_to_lineBC_distance)/Mblue));

fprintf('The XCG for blue is: %.4f m.\n',Xblue);
fprintf('The YCG for blue is: %.4f m.\n',Yblue);

figure(3);
rectangle('Position',[-0.15 -0.15 0.3 0.3],'EdgeColor','b')
hold on;
grid on;
axis([-0.2 0.2 -0.2 0.2]);
axis square;
scatter(Yblue, Xblue,'filled','b');
line([0 0],[0 0.15],'Color','blue');
set(gca,'xdir','reverse') 
xlabel('Y - Body Frame (m)');
ylabel('X - Body Frame (m)');
text(0.15, 0.16, 'LED #16')
text(-0.1, 0.16, 'LED #22')
text(-0.1, -0.16, 'LED #20')
text(0.15, -0.16, 'LED #18')


% Place Black's LEDs * USER ENTERED OFFSETS
LED16x =  0.15-Xblue-0.0115;
LED16y =  0.15-Yblue-0.012;

LED22x =  0.15-Xblue-0.011;
LED22y =  -0.15-Yblue+0.0105;

LED20x =  -0.15-Xblue+0.0115;
LED20y =  -0.15-Yblue+0.011;

LED18x =  -0.15-Xblue+0.0115;
LED18y =  0.15-Yblue-0.0105;

fprintf('\nLED #16 relative to the CG: %f mm and %f mm\n',[LED16x*1000,LED16y*1000]);
fprintf('LED #18 relative to the CG: %f mm and %f mm\n',[LED18x*1000,LED18y*1000]);
fprintf('LED #20 relative to the CG: %f mm and %f mm\n',[LED20x*1000,LED20y*1000]);
fprintf('LED #22 relative to the CG: %f mm and %f mm\n',[LED22x*1000,LED22y*1000]);

scatter((LED16x+Xblue), (LED16y+Yblue), 'filled','b');
scatter(-(LED22x+Xblue), -(LED22y+Yblue), 'filled','o');
scatter(-(LED18x+Xblue),-(LED18y+Yblue), 'filled','m');
scatter((LED20x+Xblue),(LED20y+Yblue), 'filled','g');

set(gca,'FontSize',12,'FontName','Times');  
width = 4;                                                                            
height = 4;                                                                         
pos = get(gcf, 'Position');                                               
set(gcf, 'Position', [pos(1) pos(2) width*100, height*100]); 
set(gcf, 'Paperposition', [0 0 width height])        
set(gcf,'papersize',[width height])                            
print('CG_BLUE','-dpdf','-r700'); 




% Calculate the thruster locations relative to the CG:
Thr1B = 0.15-0.081-Yblue;
Thr2B = -(0.15-0.081+Yblue);
Thr3B = 0.15-0.080-Xblue;
Thr4B = -(0.15-0.0785+Xblue);
Thr5B = 0.15-0.081+Yblue;
Thr6B = -(0.15-0.0886-Yblue);
Thr7B = 0.15-0.084+Xblue;
Thr8B = -(0.15-0.087-Xblue);


thruster_dist2CG_BLUE  = [ Thr1B
                           Thr2B
                           Thr3B
                           Thr4B
                           Thr5B
                           Thr6B
                           Thr7B
                           Thr8B ].*1000;

% Calculating Black's inertia using the bifilar pendulum * USER ENTERED VALUES
tauB = 3; % [s] period of oscillation
distance_between_cords_black = 0.28; % [m]
length_of_cord_black = 2.41805; % [m]

inertia_blue_bifilar = tauB^2 * Mblue * 9.81 * distance_between_cords_black^2 / (16*pi^2 * length_of_cord_black);
fprintf('\nBlue bifilar estimate of moment of inertia is %f kg m^2\n',inertia_blue_bifilar);

% Calculating the docking port location with respect to the centre of mass * USER ENTERED DATA *
docking_port_x = 0.15-Xblue-0.0675;
docking_port_y = 0.15-Yblue+0.075;

fprintf('\nBlue''s docking port is %.4f m in X and %.4f m in Y from its centre of mass.\n', [docking_port_x,docking_port_y]);

% Blue complete, black next

% Black with panels and an air tank * USER ENTERED DATA *
Ab = 5867/1000; % [kg] Left-middle-edge
Bb = 2755/1000; % [kg] Right-back corner
Cb = 3417/1000; % [kg] Right-front corner

Mb = Ab+Bb+Cb;

fprintf('The total mass for black is: %.4f kg.\n',Mb);

A_to_lineBC_distance = 0.3;
BC_distance = 0.3;

Xb = ((Cb-Bb)*BC_distance)/(2*Mb);
Yb = 0.15-((((Bb+Cb)*A_to_lineBC_distance)/Mb));

fprintf('The XCG for black is: %.4f m.\n',Xb);
fprintf('The YCG for black is: %.4f m.\n',Yb);

figure(1);
rectangle('Position',[-0.15 -0.15 0.3 0.3])
hold on;
grid on;
axis([-0.2 0.2 -0.2 0.2]);
axis square;
scatter(Yb, Xb,'filled','k');
line([0 0],[0 0.15],'Color','black');
set(gca,'xdir','reverse') 
xlabel('Y - Body Frame (m)');
ylabel('X - Body Frame (m)');
text(0.15, 0.16, 'LED #13')
text(-0.1, 0.16, 'LED #11')
text(-0.1, -0.16, 'LED #9')
text(0.15, -0.16, 'LED #15')


% Place Black's LEDs * USER ENTERED OFFSETS
LED13x =  0.15-Xb-0.0115;
LED13y =  0.15-Yb-0.012;

LED11x =  0.15-Xb-0.011;
LED11y =  -0.15-Yb+0.0105;

LED9x =  -0.15-Xb+0.0115;
LED9y =  -0.15-Yb+0.011;

LED15x =  -0.15-Xb+0.0115;
LED15y =  0.15-Yb-0.0105;

fprintf('\nLED #13 relative to the CG: %f mm and %f mm\n',[LED13x*1000,LED13y*1000]);
fprintf('LED #11 relative to the CG: %f mm and %f mm\n',[LED11x*1000,LED11y*1000]);
fprintf('LED #9 relative to the CG: %f mm and %f mm\n',[LED9x*1000,LED9y*1000]);
fprintf('LED #15 relative to the CG: %f mm and %f mm\n',[LED15x*1000,LED15y*1000]);

scatter(-(LED9x+Xb), -(LED9y+Yb), 'filled','b');
scatter(LED15x+Xb, LED15y+Yb, 'filled','o');
scatter(-(LED13x+Xb),-(LED13y+Yb), 'filled','m');
scatter(LED11x+Xb,LED11y+Yb, 'filled','g');

set(gca,'FontSize',12,'FontName','Times');  
width = 4;                                                                            
height = 4;                                                                         
pos = get(gcf, 'Position');                                               
set(gcf, 'Position', [pos(1) pos(2) width*100, height*100]); 
set(gcf, 'Paperposition', [0 0 width height])        
set(gcf,'papersize',[width height])                            
print('CG_BLACK','-dpdf','-r700'); 


% Calculate the thruster locations relative to the CG:
Thr1B = 0.15-0.081-Yb;
Thr2B = -(0.15-0.081+Yb);
Thr3B = 0.15-0.080-Xb;
Thr4B = -(0.15-0.0785+Xb);
Thr5B = 0.15-0.081+Yb;
Thr6B = -(0.15-0.0886-Yb);
Thr7B = 0.15-0.084+Xb;
Thr8B = -(0.15-0.087-Xb);


thruster_dist2CG_BLACK = [ Thr1B
                           Thr2B
                           Thr3B
                           Thr4B
                           Thr5B
                           Thr6B
                           Thr7B
                           Thr8B ].*1000;

% Calculating Black's inertia using the bifilar pendulum * USER ENTERED VALUES
tauB = 3.12; % [s] period of oscillation
distance_between_cords_black = 0.28; % [m]
length_of_cord_black = 2.529; % [m]

inertia_black_bifilar = tauB^2 * Mb * 9.81 * distance_between_cords_black^2 / (16*pi^2 * length_of_cord_black);
fprintf('\nBlack bifilar estimate of moment of inertia is %f kg m^2\n',inertia_black_bifilar);

% Calculating the docking port location with respect to the centre of mass * USER ENTERED DATA *
docking_port_x = 0.15-Xb-0.0675;
docking_port_y = 0.15-Yb+0.075;
 
fprintf('\nBlack''s docking port is %.4f m in X and %.4f m in Y from its centre of mass.\n', [docking_port_x,docking_port_y]);
                       
% Black complete; Red next



% Red Configuration #1: NO panels; YES Air tank; NO arm; YES shoulder motor; NO wheel
Ar = 5442/1000; % [kg] Left-middle-edge
Br = 2343/1000; % [kg] Right-back corner
Cr = 3426/1000; % [kg] Right-front corner

% Red Configuration #2: NO panels; YES Air tank; NO arm; NO shoulder motor; NO wheel
% Ar = 5056/1000; % [kg] Left-middle-edge
% Br = 2459/1000; % [kg] Right-back corner
% Cr = 3361/1000; % [kg] Right-front corner

% Red Configuration #3: YES panels; YES Air tank; NO arm; NOshoulder motor; NO wheel
% Ar = 6067/1000; % [kg] Left-middle-edge
% Br = 2953/1000; % [kg] Right-back corner
% Cr = 3714/1000; % [kg] Right-front corner

% Red Configuration #4: YES panels; YES Air tank; NO arm; YES shoulder motor; NO wheel
% Ar = 6465/1000; % [kg] Left-middle-edge
% Br = 2843/1000; % [kg] Right-back corner
% Cr = 3764/1000; % [kg] Right-front corner


Mr = Ar+Br+Cr;

fprintf('\nThe total mass for red is: %.4f kg.\n',Mr);

A_to_lineBC_distance = 0.3;
BC_distance = 0.3;

Xr = ((Cr-Br)*BC_distance)/(2*Mr);
Yr = 0.15-((((Br+Cr)*A_to_lineBC_distance)/Mr));

fprintf('The XCG for red is: %.4f m.\n',Xr);
fprintf('The YCG for red is: %.4f m.\n',Yr);

figure(2);
rectangle('Position',[-0.15 -0.15 0.3 0.3],'EdgeColor','r')
hold on;
grid on;
axis([-0.2 0.2 -0.2 0.2]);
axis square;
scatter(Yr,Xr, 'filled','k');
line([0 0],[0 0.15],'Color','red');
set(gca,'xdir','reverse') 
xlabel('Y - Body Frame (m)');
ylabel('X - Body Frame (m)');
text(0.15, 0.16, 'LED #5')
text(-0.1, 0.16, 'LED #3')
text(-0.1, -0.16, 'LED #1')
text(0.15, -0.16, 'LED #7')


% Place LEDs * USER ENTERED DATA *
LED5x =  0.15-Xr-0.01;
LED5y =  0.15-Yr-0.0105;

LED3x =  0.15-Xr-0.01;
LED3y =  -0.15-Yr+0.01;

LED1x =  -0.15-Xr+0.0095;
LED1y =  -0.15-Yr+0.01;

LED7x =  -0.15-Xr+0.011;
LED7y =  0.15-Yr-0.01;

fprintf('\nLED #5 relative to the CG: %f mm and %f mm\n',([LED5x*1000,LED5y*1000]));
fprintf('LED #3 relative to the CG: %f mm and %f mm\n',([LED3x*1000,LED3y*1000]));
fprintf('LED #1 relative to the CG: %f mm and %f mm\n',([LED1x*1000,LED1y*1000]));
fprintf('LED #7 relative to the CG: %f mm and %f mm\n',([LED7x*1000,LED7y*1000]));

scatter(-(LED1x+Xr), -(LED1y+Yr), 'filled','b');
scatter(LED7x+Xr, LED7y+Yr, 'filled','o');
scatter(-(LED5x+Xr),-(LED5y+Yr), 'filled','m');
scatter(LED3x+Xr,LED3y+Yr, 'filled','g');

set(gca,'FontSize',12,'FontName','Times');  
width = 4;                                                                        
height = 4;                                                                       
pos = get(gcf, 'Position');                                               
set(gcf, 'Position', [pos(1) pos(2) width*100, height*100]);
set(gcf, 'Paperposition', [0 0 width height])        
set(gcf,'papersize',[width height])                           
print('CG_RED','-dpdf','-r700');   

% Thruster offsets from centre of mass * USER ENTERED DATA *
Thr1R = 0.15-0.085-Yr;
Thr2R = -(0.15-0.089+Yr);
Thr3R = 0.15-0.0855-Xr;
Thr4R = -(0.15-0.083+Xr);
Thr5R = 0.15-0.0845+Yr;
Thr6R = -(0.15-0.086-Yr);
Thr7R = 0.15-0.0855+Xr;
Thr8R = -(0.15-0.0835-Xr);

thruster_dist2CG_RED   = [ Thr1R
                           Thr2R
                           Thr3R
                           Thr4R
                           Thr5R
                           Thr6R
                           Thr7R
                           Thr8R ].*1000;
                       
                       
% Calculating Red's inertia using the bifilar pendulum * USER ENTERED VALUES

% Red Configuration #1: NO panels; Air tank; NO arm; YES shoulder motor; NO wheel
tauR = 3.0822; % [s] period of oscillation
length_of_cord_red = 2.566; % [m]
% Red Configuration #2: NO panels; Air tank; NO arm; NO shoulder motor; NO wheel
% tauR = 2.9467; % [s] period of oscillation
% length_of_cord_red = 2.495; % [m]
% Red Configuration #3: YES panels; Air tank; NO arm; NO shoulder motor; NO wheel
% tauR = 2.9778; % [s] period of oscillation
% length_of_cord_red = 2.49; % [m]
% Red Configuration #4: YES panels; Air tank; NO arm; YES shoulder motor; NO wheel
% tauR = 3.0667; % [s] period of oscillation
% length_of_cord_red = 2.49; % [m]


distance_between_cords_red = 0.28; % [m]

inertia_red_bifilar = tauR^2 * Mr * 9.81 * distance_between_cords_red^2 / (16*pi^2 * length_of_cord_red);
fprintf('\nRed bifilar estimate of moment of inertia is %f kg m^2\n',inertia_red_bifilar);


% Calculating the arm mount location with respect to the centre of mass * USER ENTERED DATA *
% To clarify: this is the instantaneous centre of the first link (i.e., the
% first motor's axis of rotation)
shoulder_x = 0.15-Xr-0.0675;
shoulder_y = 0.15-Yr+0.08775;
b0 = norm([shoulder_x,shoulder_y]);
phi = atan2d(shoulder_y,shoulder_x);

fprintf('\nRed''s shoulder (IC for first link) is %.4f m in X and %.4f m in Y from its centre of mass.\n', [shoulder_x,shoulder_y]);
fprintf('Alternatively, B0 = %.4f m and phi = %.4f deg\n', [b0, phi]);


% Red properties complete, calculating arm properties next

% Bicep properties
A_bi = 122/1000; % [kg] mass measured at root of bicep segment
B_bi = 223/1000; % [kg] mass measured at end of bicep segment
L_bi = 0.3045; % [m] length of segment

M_bi = A_bi + B_bi;
b1 = A_bi*L_bi/M_bi;
a1 = L_bi - b1;
% Calculating inertia using the bifilar pendulum * USER ENTERED VALUES
tau_bi = 2.2689; % [s] period of oscillation
distance_between_cords_bi = 0.28; % [m]
length_of_cord_bi = 2.335; % [m]
inertia_bi_bifilar = tau_bi^2 * M_bi * 9.81 * distance_between_cords_bi^2 / (16*pi^2 * length_of_cord_bi);
fprintf('\nBicep mass: %f kg; a1: %f m; b1: %f m; inertia: %f kgm^2\n',[M_bi, a1, b1, inertia_bi_bifilar])


% Forearm properties
A_fa = 117/1000; % [kg] mass measured at root of bicep segment
B_fa = 218/1000; % [kg] mass measured at end of bicep segment
L_fa = 0.3045; % [m] length of segment

M_fa = A_fa + B_fa;
b2 = A_fa*L_fa/M_fa;
a2 = L_fa - b2;
% Calculating inertia using the bifilar pendulum * USER ENTERED VALUES
tau_fa = 2.24; % [s] period of oscillation
distance_between_cords_fa = 0.28; % [m]
length_of_cord_fa = 2.335; % [m]
inertia_fa_bifilar = tau_fa^2 * M_fa * 9.81 * distance_between_cords_fa^2 / (16*pi^2 * length_of_cord_fa);
fprintf('Forearm mass: %f kg; a2: %f m; b2: %f m; inertia: %f kgm^2\n',[M_fa, a2, b2, inertia_fa_bifilar])


% End-effector properties
A_ee = 32/1000; % [kg] mass measured at root of bicep segment
B_ee = 79/1000; % [kg] mass measured at end of bicep segment
L_ee = 0.08725; % [m] length of segment

M_ee = A_ee + B_ee;
b3 = A_ee*L_ee/M_ee;
a3 = L_ee - b3;
% Calculating inertia using the bifilar pendulum * USER ENTERED VALUES
tau_ee = 2.205; % [s] period of oscillation
distance_between_cords_ee = 0.075; % [m]
length_of_cord_ee = 1.78; % [m]
inertia_ee_bifilar = tau_ee^2 * M_ee * 9.81 * distance_between_cords_ee^2 / (16*pi^2 * length_of_cord_ee);
fprintf('End-effector mass: %f kg; a3: %f m; b3: %f m; inertia: %f kgm^2',[M_ee, a3, b3, inertia_ee_bifilar])


% Done with arm measurements. Printing specific things to screen for easier
% transcribing to Matlab

fprintf('\n\n\nCOPY THE FOLLOWING TO phasespace_functions.cpp\n');
fprintf('tracker_id_RED_5_pos_string = "pos=%f,%f,0";\n',[LED5x*1000,LED5y*1000]);
fprintf('tracker_id_RED_3_pos_string = "pos=%f,%f,0";\n',[LED3x*1000,LED3y*1000]);
fprintf('tracker_id_RED_1_pos_string = "pos=%f,%f,0";\n',[LED1x*1000,LED1y*1000]);
fprintf('tracker_id_RED_7_pos_string = "pos=%f,%f,0";\n',[LED7x*1000,LED7y*1000]);
fprintf('tracker_id_BLACK_13_pos_string = "pos=%f,%f,0";\n',[LED13x*1000,LED13y*1000]);
fprintf('tracker_id_BLACK_11_pos_string = "pos=%f,%f,0";\n',[LED11x*1000,LED11y*1000]);
fprintf('tracker_id_BLACK_9_pos_string = "pos=%f,%f,0";\n',[LED9x*1000,LED9y*1000]);
fprintf('tracker_id_BLACK_15_pos_string = "pos=%f,%f,0";\n',[LED15x*1000,LED15y*1000]);

fprintf('\n\nCOPY THE FOLLOWING TO Run_Initializer.m\n');
fprintf('model_param(1)                 = %f; %% RED Mass\n', Mr)
fprintf('model_param(2)                 = %f; %% RED Inertia\n', inertia_red_bifilar)
fprintf('model_param(3)                 = %f; %% BLACK Mass\n', Mb)
fprintf('model_param(4)                 = %f; %% BLACK Inertia\n', inertia_black_bifilar)
fprintf('model_param(5)                 = %f; %% BLUE Mass\n', Mblue)
fprintf('model_param(6)                 = %f; %% BLUE Inertia\n', inertia_blue_bifilar)


fprintf('\nAND\n');
fprintf('thruster_dist2CG_RED          = [%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f];\n',thruster_dist2CG_RED)
fprintf('thruster_dist2CG_BLACK        = [%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f];\n',thruster_dist2CG_BLACK)
fprintf('thruster_dist2CG_BLUE        = [%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f];\n',thruster_dist2CG_BLUE)
