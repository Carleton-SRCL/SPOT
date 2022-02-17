function [A_out, b_out] = return_inequality_matrix_and_vector_outer(r0, r_safe, Np, N_INEQ_OUTER, X_SIZE)

% Create the inequality matrix based on your current position:
A_out = zeros(N_INEQ_OUTER, X_SIZE);
b_out = zeros(N_INEQ_OUTER,1);

% Loop through, setting the values:
% Set the start index (end of the input vector):
last_index = 3;

for iRow = 1:Np
    % Get the start index:
    start_index = last_index + 1;
    
    A_out(iRow, start_index:start_index+2) = -r0';
    b_out(iRow) = -sqrt(r0'*r0)*r_safe;
    
    % Shift until the last 0:
    last_index = start_index + 8;
end

end

