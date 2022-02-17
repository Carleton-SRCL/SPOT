function [full_lb, full_ub] = return_lb_and_ub_vectors(lb_vector, ub_vector, Np)

% initialize some vector for our upper and lower bounds:
full_lb = [];
full_ub = [];

for n = 1:Np
    % Add to them. This is only called once, so size reallocation doesn't
    % matter.
    full_lb = [full_lb ; lb_vector];
    full_ub = [full_ub ; ub_vector];
    
end

end

