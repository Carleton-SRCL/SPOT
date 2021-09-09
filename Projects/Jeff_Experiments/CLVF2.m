function [h, a_ff, r, theta] = CLVF2(r_T, w_OI, w_dot_OI, o_hat, vt_I, at_I, a, b, ka, kc, vC_T,q)
% CLVF is the function for the cascaded Lyapunov vector field.

%% WORKING OUT THE DESIRED VELOCITY

    
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
            
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%% DEFINING the speed and distance functions %%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    

        % First, for distance function g(r,theta)
            if r<a
                g = r;
            else
                g = a^2./r;
                % Using a second option for "g"...
%                 g = a;
            end
            
        % Next, for speed in the alignment direction:
            if r<a
                sa = ka*(r/a)*sin(theta);
            else
                sa = ka*(a/r)*sin(theta);
            end
    
    
        % Next, for velocity in the position direction:
            if abs(r-a) <= b
                vc = kc*(a-r)/b;
            else
                vc = kc*sign(a-r);
            end
            
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%% LASTLY, outputting the desired velocity %%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
    
        h = vc*r_hat + sa*a_hat + g*cross(w_OI,r_hat) + vt_I;
       
        
%% WORKING OUT THE FEED-FORWARD ACCELERATION:

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
    
        % For the paraemeters (r_dot and theta_dot)
%             r_dot = vc; % MATCHES PAPER
            r_dot = r_hat'*vC_T;
%             theta_dot = -sa/r + (1 - g/r)*(w_OI'*e_hat); % MATCHES PAPER
            theta_dot = -o_hat'/(sin(theta)+q) * r_hat_dot + w_OI'*e_hat;
    
        % First, for g_dot:
            if r<a
                dgdr = 1;
            else
                % g_dot = -(a^2/r^2)*vc;
                % Trying a second option...
                dgdr = -a^2./r^2;
            end
            
            g_dot = dgdr * r_dot;
            
        % Next, for sa_dot:
            if r<a
                dsadr = (ka/a)*sin(theta);
                dsadtheta = (ka*r/a)*cos(theta);
            else
                dsadr = -(ka*a/r^2)*sin(theta);
                dsadtheta = (ka*a/r)*cos(theta);
            end
    
            sa_dot = dsadr*r_dot + dsadtheta*theta_dot; % MATCHES PAPER
            
        % Last, for va_dot:
            if abs(r-a) <= b
                dvcdr = -(kc/b);
            else
                dvcdr = 0;
            end
            
            vc_dot = dvcdr*r_dot;
            
            
        % FINALLY, the final feed-forward acceleration is given by: 
            a_ff = vc_dot*r_hat + vc*r_hat_dot + sa_dot*a_hat + sa*a_hat_dot + g_dot*cross(w_OI,r_hat) ...
                        + g*cross(w_dot_OI, r_hat) + g*cross(w_OI,r_hat_dot) + at_I;
    

    
end

