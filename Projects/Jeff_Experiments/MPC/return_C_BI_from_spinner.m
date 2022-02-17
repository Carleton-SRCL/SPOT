function [C_BI_Np, x_des_future] = return_C_BI_from_spinner(theta_0, t_vec, omega_z, d_B)

    % Get the total number of steps forward that we are looking, according
    % to the time-vector:
    Np = numel(t_vec);

    % Size all of the upcoming rotation matrices:
    C_BI_Np = zeros(3, 3, Np);
    
    % Create a vector for the future desired docking position:
    x_des_future = zeros(9*Np, 1);
    
    % Solve for the rate of precession:
    theta_dot = omega_z;
    
    % Solve for the upcoming angles:
    ONE = ones(size(t_vec));
    theta_future = theta_0 * ONE + theta_dot * t_vec;
    
    % initialize the height that we will fill for the upcoming states:
    iHeightEnd = 0;
    
    % solve for the upcoming matrices and angular velocities:
    for iTime = 1:numel(t_vec)
        this_theta = theta_future(iTime);
        
       C_BI =  C3(this_theta);
       
       C_BI_Np(:,:,iTime) = C_BI;
       
       omega    = [0;
                   0;
                   omega_z];
               
       % Now, fill in the upcoming states:
       iHeightStart = iHeightEnd + 1;
       
       position = C_BI'*d_B;
       vel = C_BI' * (cross(omega, d_B));
       
       x_des_future(iHeightStart:iHeightStart+2) = [0;0;0];
       x_des_future(iHeightStart+3:iHeightStart+5) = position;
       x_des_future(iHeightStart+6:iHeightStart+8) = vel;
       
       iHeightEnd = iHeightStart + 8;
       
    end

end

