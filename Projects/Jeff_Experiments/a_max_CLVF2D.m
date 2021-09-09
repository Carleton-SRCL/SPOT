function accelerationBound = a_max_UAV(params, rotStuff)

    kc = params(1);
    ka = params(2);
    b = params(3);
    a = params(4);
    
    w_max = rotStuff(1);
    w_dot_max = rotStuff(2);
    a_vehicle_max = rotStuff(3);

    accelerationBound = sqrt((kc^2/b + (ka+a*w_max)^2/a)^2 + (ka^2/(2*a) + ka*w_max + a*w_dot_max)^2) + a_vehicle_max;

end