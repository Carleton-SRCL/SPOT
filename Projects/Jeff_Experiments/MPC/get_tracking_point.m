function trackingPoint = get_tracking_point(o_hat_prime, d, r_safe)

% First, get distance from the docking port, along the docking axis,
% until we hit the safe distance:

x = -d'*o_hat_prime + sqrt((d'*o_hat_prime) + (r_safe^2 - d'*d));
trackingPoint = d + o_hat_prime*x;

end

