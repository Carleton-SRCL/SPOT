function [ A ] = C2( y )
%C2 gives the rotation matrix about the y-axis for an angle 'y'
%   y - the angle about the y-axis which you rotate
%   A - the rotation matrix

A = [cos(y) 0 -sin(y); 0 1 0 ; sin(y) 0 cos(y)];
end