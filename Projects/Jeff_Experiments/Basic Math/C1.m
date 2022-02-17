function [ A ] = C1( x )
%C1 gives the rotation matrix about the x-axis for an angle 'x'
%   x - the angle about the x-axis which you rotate
%   A - the rotation matrix

A = [1 0 0; 0 cos(x) sin(x) ; 0 -sin(x) cos(x)];
end

