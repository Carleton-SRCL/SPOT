function [d2Cdp2LVF] = costHessianLVF_heur(W_LVF, vmax, alphap, s, dSdp,finalAngle,theta_d,wandamax,dRotNorm, mu)
%COSTHESSIANLVF 
    
    % W_LVF = [W_t, W_f]
    
        W_t = W_LVF(1);
        W_f = W_LVF(2);
        
    % Getting the performance hessian:
        d2Cp_dp2 = perform_cost_hess_heur_LVF(W_f,W_t,alphap,dRotNorm,finalAngle,vmax,wandamax);
    
        
    % Now, get the Hessian for the constraint cost:
        % First, add up the dSdp's from before:
        
        constraintHessian = zeros(1,1); % Just a number.
        
        for k = 1:numel(s) % Three constraints!
            constraintHessian = constraintHessian + dSdp(k,:)'*dSdp(k,:);
        end
        
    % Lastly, add in the Hessian from the upper acceleration constraint:
        secondDerPart = -hess_p_amaxLVF_heur(alphap,finalAngle,theta_d)/s(end);
        
    % Add up to get the total Hessian:
        d2Cdp2LVF = mu*d2Cp_dp2 + (constraintHessian - secondDerPart)./numel(s); % MINUS THE SECOND PART?

end

