function accelerationBound = a_max_LVF(params, rotStuff)

    v_max = params(1);
    a_prime = params(2);
    
    w_max = rotStuff(1);
    theta_d = rotStuff(2);
    d = [rotStuff(3);rotStuff(4);rotStuff(5)];

    d_norm = sqrt(sum(d.^2));
    
    accelerationBound = sqrt(  ...
                            (pi/theta_d*v_max^2/a_prime + 2*v_max^2/a_prime + 2*v_max*w_max + w_max^2*a_prime)^2  ...
                            +...
                            (v_max^2*(1+pi/(2*theta_d)) + 2*w_max*v_max + v_max^2 )^2  ...
                        ) + w_max^2*d_norm;

end