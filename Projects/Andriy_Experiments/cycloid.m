function [x_ref,F,jerk] = cycloid(t,R,H,w,x0,y0,phi,rotation)

%NOTE: phase shift behaviour "phi"
%has not been dealt with in derivatives yet.
% rotation of xy shape also not dealt with in derivatives.

K = 3;

%I know it seems silly putting phi in the brackets, but it allows us to
%change the track starting position without changing ANYTHING ELSE
x = R*cos(w*(t+phi)) + H*sin(K*w*(t+phi)) + x0;
y = R*sin(w*(t+phi)) + H*cos(K*w*(t+phi)) + y0;
z = zeros(1,length(t));

%Include  rotation (so that it fits on the table:

xnew = cos(rotation).*x + sin(rotation).*y;
ynew = -sin(rotation).*x + cos(rotation).*y;

x = xnew;
y = ynew;

xd = -w*R*sin(w*t) + K*w*H*cos(K*w*t);
yd = w*R*cos(w*t) - K*w*H*sin(K*w*t);
zd = zeros(1,length(t));

xdd = -w^2*R*cos(w*t) - K^2*w^2*H*sin(K*w*t);
ydd = -w^2*R*sin(w*t) - K^2*w^2*H*cos(K*w*t);
zdd = zeros(1,length(t));

xddd = w^3*R*sin(w*t) - K^3*w^3*H*cos(K*w*t);
yddd = -w^3*R*cos(w*t) + K^3*w^3*H*sin(K*w*t);
zddd = zeros(1,length(t));

x_ref = [x;y;z;xd;yd;zd]';
F = [xdd;ydd;zdd]';
jerk = [xddd;yddd;zddd]';

end

