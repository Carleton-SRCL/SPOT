function [ A ] = skew( v )
%SKEW returns the skew symmetric matrix of a column matrix
%   v - the input vector
%   A - the skew symmetric matrix

A = [0 -v(3) v(2);v(3) 0 -v(1) ; -v(2) v(1) 0];
end

