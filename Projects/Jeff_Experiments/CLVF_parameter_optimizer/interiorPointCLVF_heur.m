function [b, kc, ka] = interiorPointCLVF_heur(b0, kc0, ka0, a, a_max, w_max, rotNorm, rC_T0, vC_T0, W_CLVF, mu, tol, gamma, beta, muFact, muLimit)

%             tol = 10^-8;
%             mu = 100; % My weighting parameter for the perf vs. constraints.
%             gamma = 1; % My damping factor
%             beta = 0.8; % My reduction factor.

            % LEGEND OF WEIGHTS:

% W_CLVF = [W_t, W_f, W_j];

            while mu < muLimit
                [b, kc, ka] = newtonStepCLVF_heur(b0, kc0, ka0, a, a_max, w_max, rotNorm, rC_T0, vC_T0, W_CLVF, mu, tol, gamma, beta);
                mu = muFact*mu;
                b0 = b;
                kc0 = kc;
                ka0 = ka;
            end

end

