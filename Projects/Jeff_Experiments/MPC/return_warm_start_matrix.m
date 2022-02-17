function [warm_start_matrix] = return_warm_start_matrix(u_plus_x_size, Np)

    % initialize the size of the matrix:
    d = u_plus_x_size*Np;
    warm_start_matrix = zeros(d);
    
    % The top right corner is an identity matrix:
    warm_start_matrix(1:end-u_plus_x_size, u_plus_x_size+1:end) = eye(d-u_plus_x_size);
    
    % The very bottom right is a smaller identity matrix:
    warm_start_matrix(end-u_plus_x_size+1:end, end-u_plus_x_size+1:end) = eye(u_plus_x_size);

end

