function [params] = lineSearch(params0, a_limit_function, searchDirection, desiredAccuracy, a_max, stepSize, stepShrinkFactor, rotStuff)
% LINESEARCH will be used to find suitable parameters for the Lyapunov
% Docking procedure.

% INPUTS:
%   params0             :   An initial guess at the parameters.
%   a_limit_function    :   The function of the accel upper bound.
%   searchDirection     :   A vector for the direction of search.
%   desiredAccuracy     :   The accuracy level for the search.
%   a_max               :   The acceleration limit of the vehicle.
%   stepSize            :   The initial step size in the search
%   stepShrinkFactor    :   The shrink factor for the step (x < 1)

% OUTPUTS:
%   params              :   The parameters which meet the criteria.


params = params0;


currentALimit = feval(a_limit_function, params, rotStuff); % Check if initial guess is good enough
error = sqrt((currentALimit - a_max)^2);
firstRound = 1; % Set this flag UP

    while error > desiredAccuracy
        if firstRound == 1
            firstRound = 0; % Set this flag DOWN.
            searchDirectionSign = sign(a_max - currentALimit);
            params = params + stepSize*searchDirection*searchDirectionSign;
        elseif directionChanged == -1 % WE DID CHANGE DIRECTION!
            stepSize = stepShrinkFactor*stepSize;
            params = params + stepSize*searchDirection*searchDirectionSign;
        else
            params = params + stepSize*searchDirection*searchDirectionSign;
        end
        
        currentALimit = feval(a_limit_function, params, rotStuff); % Check if initial guess is good enough
        error = sqrt((currentALimit - a_max)^2);
        
        directionChanged = searchDirectionSign*sign(a_max - currentALimit);
        searchDirectionSign = sign(a_max - currentALimit);
    end
end