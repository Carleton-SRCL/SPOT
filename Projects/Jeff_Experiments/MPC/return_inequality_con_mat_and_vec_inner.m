function [P, z] = return_inequality_con_mat_and_vec_inner(u_con_mat, u_con_vec, x_con_mat, x_con_vec, Np, Gf, G_vec, C_BI_Np, A_cone)

    %% DESCRIPTION: 
    % this function produces the inequality matrix for the MPC solver.
    
    %% INPUTS:
    % u_con_mat - the matrix for the input vector inequality:
    % u_con_vec - the vector the input inequalities.
    % x_con_mat - the matrix for the state inequalities... or at least a
    %   matrix of the same size... since this will be rebuilt for every
    %   timestep, it doesn't matter much.
    % Np - the number of steps considered for the MPC horizon.
    % Gf - the 'terminal set' if one is used.
    % G_vec - also part of the terminal set, again, if one is used.
    % C_BI_Np - the C_BI estimates for the next Np steps.
    % d - the docking cone position in the body-fixed frame of the target.
    % A_cone - the matrix describing the approximated cone for docking.
    
    % NOTE - THE X-CONSTRAINT VECTOR HAS TO BE RE-SOLVED FOR EVERY SINGLE
    % TIMESTEP, SINCE IT IS BASED ON THE MAX VALUE OF X_CON_MAT*VECTOR, AND
    % X_CON_MAT CHANGES AT EVERY TIMESTEP BASED ON ROTATION.
    
    %% OUTPUTS:
    % P - the inequality matrix.
    % z - the inequality vector.
     
    % Get the total size of the U and X vectors:
    u_dim = size(u_con_mat, 2); % shouldn't hard code this lol.
    x_dim = size(x_con_mat, 2);
    
    % Get the total "constraint height" for each vector:
    n_u_consts = size(u_con_mat, 1);
    n_x_consts = size(x_con_mat, 1);
    
    % Preallocate size for the overall constraint matrix and vector:
    X_SIZE = Np* (u_dim + x_dim);    
    N_CONSTRAINTS = Np * (n_u_consts + n_x_consts);
    
    % Create the M_Vec:
    M_vec = [u_con_vec;x_con_vec];
    
    % First, check if we are including the terminal set or not (may not be
    % needed, is included in cost anyways).
    if isempty(Gf)
       P = zeros(N_CONSTRAINTS, X_SIZE);
       z = zeros(N_CONSTRAINTS, 1);
    else
        G_size = size(Gf);  
        P = zeros(N_CONSTRAINTS + G_size(1), X_SIZE);
        z = zeros(N_CONSTRAINTS + G_size(1), 1);
    end
    
    % Create a single instance of the combined U and X constraint matrix:
    % Loop through, filling in the values:
    iHeight = 1;
    iWidth = 1;
    
    for iMat = 1:Np
       % RECREATE THE X_CON MATRIX... NEEDS INPUTS FROM THE UPCOMING
       % ATTITUDE OF THE SPACECRAFT.
       C_BI = C_BI_Np(:,:,iMat);
       x_con_mat = [A_cone*C_BI     zeros(size(A_cone))];
       M_mat = [zeros(n_x_consts, u_dim), x_con_mat];         
       
       %  Get the ending indices:
       iHeightEnd = iHeight + (n_u_consts + n_x_consts - 1);
       iWidthEnd = iWidth + (u_dim + x_dim - 1);
       
       % Fill in the matrix and the vector:
       P(iHeight:iHeightEnd, iWidth:iWidthEnd) = M_mat;
       z(iHeight:iHeightEnd) = M_vec;
       
       % Increment:
       iHeight = iHeightEnd + 1;
       iWidth = iWidthEnd + 1;
    end
    
    if ~isempty(Gf) % Add in the terminal set for the inequality constraint:
        P(end-G_size(1) + 1:end, end-G_size(2) + 1:end) = Gf;
        z(end-G_size(1) + 1:end) = G_vec;
    end
    
    
end

