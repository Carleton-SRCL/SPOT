function [a_prime, v_max] = interiorPointLVF(a_prime0, v_max0, a_max, w_max, rotNorm, dockingPortNorm, theta_d, fact, W_LVF, mu, tol, gamma, beta, muFact, muLimit)
    

%         tol = 10^-8;
%         mu = 100; % My weighting parameter for the perf vs. constraints.
%         gamma = 1; % My damping factor
%         beta = 0.8; % My reduction factor.
        
        % LEGEND OF WEIGHTS:
%         % For time:
%             W_t_vmax = W_LVF(1);
% 
%         % For fuel:
%             W_f_alphap = W_LVF(2);
%             W_f_vmax = W_LVF(3);
% 
%         % For risk:
%             W_r = W_LVF(4);

%         W_LVF = [1, 1, 100, 1000];

        while mu < muLimit
            [a_prime, v_max] = newtonStepLVF(a_prime0, v_max0, a_max, w_max, rotNorm, dockingPortNorm, theta_d, fact, W_LVF, mu, tol, gamma, beta);
    
            mu = muFact*mu;
            a_prime0 = a_prime;
            v_max0 = v_max;
        end

end

