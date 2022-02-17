function innerAccelerationBound = a_max_LVF3D(params, rotStuff)
% A = PARAMS(1)
% v = PARAMS(2)
    %   N1: dockingPortNorm
    %   N2: rotNorm
    %   N3: w_max for Coriolis stuff

v = params(1);
A = params(2);


dockingPortNorm = rotStuff(1);
rotNorm = rotStuff(2);
w_max = rotStuff(3);
theta_d = rotStuff(4);
fact = rotStuff(5);

% innerAccelerationBound = pi/A*v^2*(1 + pi/(2*theta_d)) + 2*w_max*v + dockingPortNorm + rotNorm*A;
innerAccelerationBound = 0.724612*(1 + pi/(2*theta_d))*(v^2*fact*pi/A)+ 2*w_max*v + dockingPortNorm + rotNorm*A;

end