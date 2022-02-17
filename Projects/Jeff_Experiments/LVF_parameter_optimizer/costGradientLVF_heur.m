function [dCdp, dSdp] = costGradientLVF_heur(alphap, vmax, ...
    W_LVF, theta_d, wandamax, wmax, dRotNorm, finalAngle, s, mu)
    % Break W out of its vector:
    
    % W_LVF = [W_t, W_f]
    
    W_t = W_LVF(1);
    W_f = W_LVF(2);
            
    % Total performance cost gradient:
        
        dCp_dp = perform_cost_grad_heur_LVF(W_f,W_t,alphap,dRotNorm,finalAngle,vmax,wandamax);        
            
    % COST GRADIENT FOR SLACKS:  
        dSdp = zeros(2, 1);
        
        dSdp(1,1) = 1/s(1);
        
        dSdp(2,1) = -grad_p_amaxLVF_heur(alphap,finalAngle,theta_d,vmax,wmax)/s(2);
        
    % Total slack cost gradient:
        dConstraints_dp = transpose(sum(dSdp));
        
    % TOTAL cost gradient:
        dCdp = mu*dCp_dp - dConstraints_dp/numel(s);
end

