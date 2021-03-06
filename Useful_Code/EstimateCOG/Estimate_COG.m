% Estimate CG using 3 scales.

clear;
clc;
close('all')

Ab = 5553.80/1000; % Front where the port is
Bb = 2853.40/1000; % Right, where the power switch is
Cb = 3926.90/1000; % Left, opposite from the power switch

Mb = Ab+Bb+Cb;

Ab = Ab*9.81;
Bb = Bb*9.81;
Cb = Cb*9.81;

Wb = Ab+Bb+Cb;

fprintf('The total mass for black is: %.4f kg.\n',Mb);

Lb = 0.3;
Db = 0.3;

Yb = 0.15-((((Bb+Cb)*Lb)/Wb));
Xb = ((Cb-Bb)*Db)/(2*Wb);

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
text(0.15, 0.16, 'LED #15')
text(-0.1, 0.16, 'LED #9')
text(-0.1, -0.16, 'LED #11')
text(0.15, -0.16, 'LED #13')

% Place LEDs

LED15x =  0.15-Xb-0.011;
LED15y =  0.15-Yb-0.0115;

LED9x =  0.15-Xb-0.012;
LED9y =  -0.15-Yb+0.01;

LED11x =  -0.15-Xb+0.0095;
LED11y =  -0.15-Yb+0.0105;

LED13x =  -0.15-Xb+0.01125;
LED13y =  0.15-Yb-0.0105;

fprintf('\nLED #15 relative to the CG: %f mm and %f mm\n',[LED15x*1000,LED15y*1000]);
fprintf('\nLED #9 relative to the CG: %f mm and %f mm\n',[LED9x*1000,LED9y*1000]);
fprintf('\nLED #3 relative to the CG: %f mm and %f mm\n',[LED11x*1000,LED11y*1000]);
fprintf('\nLED #4 relative to the CG: %f mm and %f mm\n',[LED13x*1000,LED13y*1000]);

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

Thr1B = 0.15-0.0815-Yb;
Thr2B = -(0.15-0.0825+Yb);
Thr3B = 0.15-0.081-Xb;
Thr4B = -(0.15-0.103+Xb);
Thr5B = 0.15-0.081+Yb;
Thr6B = -(0.15-0.111-Yb);
Thr7B = 0.15-0.086+Xb;
Thr8B = -(0.15-0.081-Xb);

thruster_dist2CG_BLACK = [ Thr1B
                           Thr2B
                           Thr3B
                           Thr4B
                           Thr5B
                           Thr6B
                           Thr7B
                           Thr8B ].*1000;



% Weight with 1 links at pi/2 
Ar1 = 9587.85/1000; % Front where the port is
Br1 = 4218.30/1000; % Right, where the power switch is
Cr1 = 3431.90/1000; % Left, opposite from the power switch

Mr1 = 273.3/1000;

% Weight with 2 links at pi/2 
Ar2 = 9782.05/1000; % Front where the port is
Br2 = 4170.10/1000; % Right, where the power switch is
Cr2 = 3564.00/1000; % Left, opposite from the power switch

Mr2 = 263.1/1000;

% Weight with 3 links at pi/2 
Ar3 = 9788.55/1000; % Front where the port is
Br3 = 4577.50/1000; % Right, where the power switch is
Cr3 = 3240.00/1000; % Left, opposite from the power switch

Mr3 = 103.7/1000;

% Weight with 0 links
Ar = 9230.75/1000; % Front where the port is
Br = 4218.30/1000; % Right, where the power switch is
Cr = 3431.90/1000; % Left, opposite from the power switch

Mr = mean([Ar+Br+Cr,Ar1+Br1+Cr1-Mr1,Ar2+Br2+Cr2-Mr2-Mr1,Ar3+Br3+Cr3-Mr3-Mr2-Mr1]);

fprintf('\nThe total mass for red is: %.4f kg.\n',Mr);

Wr = Mr*9.81;

Lr = 0.3;
Dr = 0.3;

Ar = Ar*9.81;
Br = Br*9.81;
Cr = Cr*9.81;

Yr = 0.15-((((Br+Cr)*Lr)/Wr));
Xr = ((Cr-Br)*Dr)/(2*Wr);

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
text(0.15, 0.16, 'LED #7')
text(-0.1, 0.16, 'LED #1')
text(-0.1, -0.16, 'LED #3')
text(0.15, -0.16, 'LED #5')

% Place LEDs

LED7x =  0.15-Xr-0.01;
LED7y =  0.15-Yr-0.0105;

LED1x =  0.15-Xr-0.012;
LED1y =  -0.15-Yr+0.0105;

LED3x =  -0.15-Xr+0.01;
LED3y =  -0.15-Yr+0.011;

LED5x =  -0.15-Xr+0.0115;
LED5y =  0.15-Yr-0.011;

fprintf('\nLED #7 relative to the CG: %f mm and %f mm\n',([LED7x*1000,LED7y*1000]));
fprintf('\nLED #1 relative to the CG: %f mm and %f mm\n',([LED1x*1000,LED1y*1000]));
fprintf('\nLED #5 relative to the CG: %f mm and %f mm\n',([LED3x*1000,LED3y*1000]));
fprintf('\nLED #3 relative to the CG: %f mm and %f mm\n',([LED5x*1000,LED5y*1000]));

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

Thr1R = 0.15-0.0855-Yr;
Thr2R = -(0.15-0.0865+Yr);
Thr3R = 0.15-0.0865-Xr;
Thr4R = -(0.15-0.0795+Xr);
Thr5R = 0.15-0.0835+Yr;
Thr6R = -(0.15-0.085-Yr);
Thr7R = 0.15-0.0856+Xr;
Thr8R = -(0.15-0.081-Xr);

thruster_dist2CG_RED   = [ Thr1R
                           Thr2R
                           Thr3R
                           Thr4R
                           Thr5R
                           Thr6R
                           Thr7R
                           Thr8R ].*1000;

F_thrusters_BLACK      = 0.25.*ones(8,1);                       
F_thrusters_RED        = 0.25.*ones(8,1);                       

save('thruster_param','thruster_dist2CG_BLACK','thruster_dist2CG_RED',...
    'F_thrusters_BLACK','F_thrusters_RED');