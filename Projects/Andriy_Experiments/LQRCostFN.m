function [Cost] = LQRCostFN(THTime, THError,THInput,q,r)
%LQRCostFN Provides the LQR cost of a Timehistory
%       THTime is the total time, as a vector
%       THError is the timehistory of the error as an size(THTime)*m matrix
%       THInput is the control activation timehistory as an size(THTime)*n
%       matrix, and must be in common time with THError and THTime.
%       q is the matrix of error weights, m*m
%       r is the matrix of activation weights, n*n

%The cost function is:

%J = integral from 0 to T of (q * THError^2 + r * THInput^2)

%Each individual term, through matrices:
costTerms(1) = (THError(1,:)) * q * (THError(1,:)') + (THInput(1,:))*r*(THInput(1,:)');
for i = 2:length(THTime)
    costTerms(i) =  (THError(i,:)) * q * (THError(i,:)') + (THInput(i,:))*r*(THInput(i,:)');
    %Integrate through time:
    %rectangular integration: I from y1 to y2 = deltaX * (y1+y2)/2;
    integral(i) = (THTime(i)-THTime(i-1))*0.5*(costTerms(i)+costTerms(i-1));
end
Cost = sum(integral);
end

