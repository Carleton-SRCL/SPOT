function [s] = computeSlacksCLVF_quad(b, kc, ka, alpha,... 
    amax, wmax, wandamax)

    % Returns slack variables if all constraints are met. Otherwise,
    % returns an empty vector.
    
    s = zeros(4,1);
    
    s(1) = b; s(2) = kc; s(3) = ka; 
    
    paramsCLVF = [ka, kc, b, alpha];
    
    rotStuffCLVF = [wandamax, wmax];

    s(4) = amax - a_max_CLVF3D_quad(paramsCLVF, rotStuffCLVF);
    
    if ~isempty(find(s<=0, 1))
        s = []; % Give back blank vector.
    end
end

