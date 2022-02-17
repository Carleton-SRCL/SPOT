function [s] = computeSlacksLVF_heur(alphap, vmax,... 
    amax, wmax, wandamax, dockingPortNorm, theta_d, fact)

    % Returns slack variables if all constraints are met. Otherwise,
    % returns an empty vector.
    
    s = zeros(2,1);
    
    s(1) = vmax;
    
    paramsLVF = [vmax, alphap];
    rotStuffLVF  = [dockingPortNorm, wandamax, wmax, theta_d, fact];

    s(2) = amax - a_max_LVF3D(paramsLVF, rotStuffLVF);
    
    if ~isempty(find(s<=0, 1))
        s = []; % Give back blank vector.
    end
end