function [b, kc, ka] = newtonStepCLVF_heur(b0, kc0, ka0, alpha, amax, wmax, wandamax,rC_T0, vC_T0, W_CLVF, mu, tol, gamma, beta)

    % FOR A GIVEN MU VALUE WHICH WILL CHANGE IN OUTER LOOP.
    
    % Get the slack variables:
        s0 = computeSlacksCLVF_quad(b0, kc0, ka0, alpha,... 
                                amax, wmax, wandamax);
                            
        if isempty(s0)
           disp("Invalid start point");
           return;
        end
                            
    cnt = 1; % Counts the step we're on.

    while 1 % Go on forever.
    
        % First, check gradient:
            [dCdp, dSdp0] = costGradientCLVF_heur(alpha,b0,ka0,kc0, ...
                                                W_CLVF,wmax,wandamax,rC_T0, vC_T0, s0, mu);

            stdGrad = sqrt(sum(dCdp.^2));
            
        % Stop ourselves if we are stuck:
            if cnt >= 1000
                disp("safe break");
                break;
            end

        % Check if we're already good enough:
        
        if stdGrad <= tol % We ARE good enough.
            
            if cnt == 1 % The first step:
                % We're already at a minimum!
                b = b0;
                kc = kc0;
                ka = ka0;
            end
            
            disp("Found minimum");
            break;
            
        else % We are NOT good enough :(
            
            % Compute Hessian, the compute step.
                dC2dp2 = costHessianCLVF_heur(alpha,b0,ka0,kc0,rC_T0,vC_T0,wandamax, W_CLVF, s0, dSdp0,wmax, mu);
                xStep = -dC2dp2\dCdp;
            
            % Take the step:
                newParams = [b0;kc0;ka0] + gamma*xStep;
                b = newParams(1);
                kc = newParams(2);
                ka = newParams(3);
                
            % Recompute the slack variable - if we left the bounds, then
            % dampen!
                s = computeSlacksCLVF_quad(b, kc, ka, alpha,... 
                                        amax, wmax, wandamax);
                                    
%                 C = costFunctionCLVF_quad(b,kc, ka, s, W_CLVF, mu);
                
            while isempty(s)% || C >= C0
                % Smaller step. Try it all again.
                    xStep = beta*xStep; 
                % Take the step:
                    newParams = [b0;kc0;ka0] + gamma*xStep;
                    b = newParams(1);
                    kc = newParams(2);
                    ka = newParams(3);
                
            % Recompute the slack variable - if we left the bounds, then
            % dampen!
                s = computeSlacksCLVF_quad(b, kc, ka, alpha,... 
                                        amax, wmax, wandamax);                                   
            end
            
        end

        % RESET:
            s0 = s; b0 = b; kc0 = kc; ka0 = ka;
            cnt = cnt+1;
    end
    
end

