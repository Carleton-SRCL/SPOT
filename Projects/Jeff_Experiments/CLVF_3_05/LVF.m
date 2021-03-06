function [h, a_ff, r, theta] = LVF(r_T, o_hat, q, k_v_rel, theta_d, w_OI, vt_I, a_prime, r_co, vC_T, at_I, w_dot_OI)

%% DESIRED VELOCITY
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%% FIRST, defining r and theta %%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
    
        % For r:
            r = sqrt(sum(r_T.^2));
            
        % For theta:
        % Just in case, work out a magnitude for o_hat as well...?
            o_mag = sqrt(sum(o_hat.^2));
            theta = acos(r_T'*o_hat/(r*o_mag));
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%% THEN, defining our directions %%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        
        % For the r_hat direction:
            r_hat = r_T./r;
        
        % For the a_hat direction:
            a_hat = (o_hat - r_hat.*cos(theta))./(sin(theta)+q);
            
            
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%% LASTLY, defining our middle variables %%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        
        % Get theta_N
            if theta < theta_d
                theta_N = (theta/theta_d)*pi/2;
            else
                theta_N = pi/2;
            end
            
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%% DEFINING the speed and distance functions %%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    

        % First, for distance function g ==> just "r" for a LVF.
            g = r;
            
        % The relative speed function:
            if r < a_prime/2
                v_rel = k_v_rel*r;
            elseif r < r_co
                v_rel = k_v_rel*a_prime/2 - k_v_rel*(r-a_prime/2);
            else
                v_rel = k_v_rel*a_prime/2 - k_v_rel*(r_co-a_prime/2); % CONSTANT.
            end
            
        % Next, for speed in the alignment direction:
            sa = v_rel*sin(theta_N);
    
        % Next, for velocity in the position direction:
            vc = -v_rel*cos(theta_N);
            
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%% LASTLY, outputting the desired velocity %%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
    
        h = vc*r_hat + sa*a_hat + g*cross(w_OI,r_hat) + vt_I;
        
%% FEED-FORWARD ACCELERATIONS:

    e_hat = cross(r_hat,a_hat);

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%% FIRST, defining feed-forward accels %%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 

        % For the direction vectors:
%             r_hat_dot = (sa*a_hat + g*cross(w_OI,r_hat))/r; % THIS WAS A RATHER THAN A_HAT % MATCHES PAPER
            r_hat_dot = (eye(3,3) - r_hat*r_hat')/r * vC_T;
            
%             a_hat_dot_PART1 = (- sa - g*w_OI'*e_hat)/(r)   *   r_hat;
%             a_hat_dot_PART2 = (g*cos(theta)*w_OI'*a_hat)/(r*sin(theta))   *  e_hat;
%             a_hat_dot_PART3 = (w_OI'*(r_hat*sin(theta) - a_hat*cos(theta)))/(sin(theta))   *   e_hat;
%             
%             a_hat_dot_PART2 = zeros(3,1);
%             a_hat_dot_PART3 = zeros(3,1);
%             
%             a_hat_dot = a_hat_dot_PART1 + a_hat_dot_PART2 + a_hat_dot_PART3;
           d_a_hat_d_r_hat = (eye(3,3) - a_hat*a_hat')/(a_hat'*o_hat+q)  *  (-r_hat*o_hat' - (r_hat'*o_hat)*eye(3,3));
           d_a_hat_d_o_hat = e_hat*e_hat'/(a_hat'*o_hat + q);
           
           a_hat_dot = d_a_hat_d_r_hat*r_hat_dot + d_a_hat_d_o_hat*cross(w_OI,o_hat);
    
        % For the parameters (r_dot and theta_dot)
%             r_dot = vc; % MATCHES PAPER
            r_dot = r_hat'*vC_T;
%             theta_dot = -sa/r + (1 - g/r)*(w_OI'*e_hat); % MATCHES PAPER
            theta_dot = -o_hat'/(sin(theta)+q) * r_hat_dot + w_OI'*e_hat;
    
        % First, for g_dot:
            dgdr = 1; % Since g == r inside the cutoff radius.
            g_dot = dgdr * r_dot;
            
            
        % For va_dot:
            if r < a_prime/2
                dvcdr = -k_v_rel*cos(theta_N);
            elseif r < r_co
                dvcdr = k_v_rel*cos(theta_N);
            else
                dvcdr = 0;
            end
            
            if theta < theta_d
                dvcdtheta = v_rel*sin(theta_N) * pi/(2*theta_d);
            else
                dvcdtheta = 0;
            end
            
            vc_dot = dvcdr*r_dot + dvcdtheta*theta_dot;
            
        % Next, for sa_dot:
            if r<a_prime/2
                dsadr = k_v_rel*sin(theta_N);
            elseif r < r_co
                dsadr = -k_v_rel*sin(theta_N);
            else
                dsadr = 0;
            end
            
            if theta < theta_d
                dsadtheta = v_rel*cos(theta_N) * pi/(2*theta_d);
            else
                dsadtheta = 0;
            end
    
            sa_dot = dsadr*r_dot + dsadtheta*theta_dot; % MATCHES PAPER
            
            
        % FINALLY, the final feed-forward acceleration is given by: 
            a_ff = vc_dot*r_hat + vc*r_hat_dot + sa_dot*a_hat + sa*a_hat_dot + g_dot*cross(w_OI,r_hat) ...
                        + g*cross(w_dot_OI, r_hat) + g*cross(w_OI,r_hat_dot) + at_I;

end

