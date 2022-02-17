function [vmax] = newtonStepLVF_heur(alphap, vmax0, amax, wmax, wandamax, dockingPortNorm, theta_d, fact, W_LVF, mu, tol, gamma, beta)

    % FOR A GIVEN MU VALUE WHICH WILL CHANGE IN OUTER LOOP.
    
    finalAngle = fact*pi; 
    
    % Get the slack variables:
        s0 = computeSlacksLVF_heur(alphap, vmax0,... 
            amax, wmax, wandamax, dockingPortNorm, theta_d, fact);
        
        if isempty(s0)
           disp("Not valid LVF");
           return;
        end
        
    cnt = 1; % Counts the step we're on.

    while 1 % Go on forever.
    
        % First, check gradient:
            %[dCdp, dSdp] = costGradientLVF_heur(alphap, vmax0, ...
            %                    W_LVF, fact,theta_d, wandamax, wmax, s0, mu);
            [dCdp, dSdp] = costGradientLVF_heur(alphap, vmax0, ...
                                W_LVF, theta_d, wandamax, wmax, dockingPortNorm, finalAngle, s0, mu);
                            
                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            %%%%%%%%%%%%%%%%%% NO CHANGES AFTER THIS!!
                            %%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            

            stdGrad = sqrt(sum(dCdp.^2));
%             disp("stdGrad is:")
%             disp(stdGrad);
            
        if cnt > 1000
            disp("Safe break.")
            break;
        end

        % Check if we're already good enough!
        
        if stdGrad <= tol % We ARE good enough!!
            
            if cnt == 1 % The first step:
                % We're already at a minimum!
                vmax = vmax0;
            end
            
            disp("Found minimum");
            break;
            
        else % We are NOT good enough :(
            
            % Compute Hessian, the compute step.
%             dC2dp2 = costHessianLVF(W_LVF, vmax0, alphap0, s0, dSdp,fact,theta_d,wandamax,wmax, mu);
              dC2dp2 = costHessianLVF_heur(W_LVF, vmax0, alphap, s0, dSdp, finalAngle, theta_d, wandamax, dockingPortNorm, mu);
            xStep = -dC2dp2\dCdp;
            
            % Take the step:
                vmax = vmax0 + gamma*xStep;
                 
                
            % Recompute the slack variable - if we left the bounds, then
            % dampen!
                s = computeSlacksLVF_heur(alphap, vmax,... 
                            amax, wmax, wandamax, dockingPortNorm, theta_d, fact);
                        
%                 C = costFunctionLVF(alphap, vmax, s, W_LVF, mu, wandamax);
                
            while isempty(s)% || C > C0
                % Smaller step. Try it all again.
                    xStep = beta*xStep; 
                % Take the step:
                    vmax = vmax0 + gamma*xStep;
                
            % Recompute the slack variable - if we left the bounds, then
            % dampen!
                s = computeSlacksLVF_heur(alphap, vmax,... 
                            amax, wmax, wandamax, dockingPortNorm, theta_d, fact);
                        
%                 C = costFunctionLVF(alphap, vmax, s, W_LVF, mu, wandamax);
            end
%             disp(xStep)
        end

        % RESET:
            s0 = s; vmax0 = vmax;
            cnt = cnt+1;
    end
    
end

