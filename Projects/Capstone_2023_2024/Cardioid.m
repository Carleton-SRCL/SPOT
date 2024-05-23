function [alpha, d] = Cardioid(state_red,state_black,shape)
% This function produces the docking cone scalar factor d(alpha) used in the 
% repulsive potential function  for the target spacecraft.
% Written by Parker Stewart
% Inputs:
%       state_red: current state of chaser spacecraft (6x1)
%       state_black: current state of target spacecraft (6x1) 
% Outputs:
%       d: distance from origin to perimeter of docking cone cardioid shape
%       (scalar)

%Size parameters
a1 = shape(1);
a2 = shape(2);
a3 = shape(3);
b1 = shape(4);
b2 = shape(5);

xc = state_red(1);
xb = state_black(1)+0.165*cos(state_black(3)); %Includes offset for docking cone
yc = state_red(2)-0.165*sin(state_black(3)); %Includes offset for docking cone
yb = state_black(2);
thetac = state_red(3);
thetab = state_black(3);

alpha_pol = wrapToPi(atan2(xc-xb,yc-yb) +pi/2 + thetab); %takes rotation into account

if (0 <= alpha_pol) && (alpha_pol < pi/2)
    d_pol = (2*b1*a1*a1*cos(alpha_pol))./(a1*a1*(cos(alpha_pol)).^2+b1*b1*(sin(alpha_pol)).^2);
elseif (pi/2 <= alpha_pol) && (alpha_pol < pi)
    d_pol = (-2*b2*a2*a2*cos(alpha_pol))./(a2*a2*(cos(alpha_pol)).^2+b2*b2*(sin(alpha_pol)).^2);
elseif (-pi <= alpha_pol) && (alpha_pol < -pi/2)
    d_pol = (2*b2*a3)./sqrt((a3*a3*(cos(alpha_pol)).^2+4*b2*b2*(sin(alpha_pol)).^2));
elseif (-pi/2 <= alpha_pol) && (alpha_pol < 0)
    d_pol = (2*b1*a3)./sqrt(a3*a3*(cos(alpha_pol)).^2+4*b2*b2*(sin(alpha_pol)).^2);
else
    d_pol = 0;
end

[x,y] = pol2cart(alpha_pol,d_pol);

M = [cos(thetab), -sin(thetab); sin(thetab), cos(thetab)]*[x, y]';

x = M(1);
y = M(2);

[alpha,d] = cart2pol(x,y);
