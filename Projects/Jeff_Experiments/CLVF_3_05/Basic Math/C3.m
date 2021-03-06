function [ A ] = C3( z )
%C3 gives the rotation matrix about the z-axis for an angle 'z'
%   z - the angle about the z-axis which you rotate
%   A - the rotation matrix

A = [cos(z) sin(z) 0; -sin(z) cos(z) 0 ; 0 0 1];
end