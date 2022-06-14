clear;
clc;

%==============================================================%
% This script derives the equations of motion of SPOT.
% Author: Alex
% Creation Date: 2016-08-04
% Last Edit:     2018-01-18
%==============================================================%

% Define symbolic variables
syms a1 a2 a3 b0 b1 b2 b3 q0 q1 q2 q3 m0 m1 m2 m3 x0_dot y0_dot q0_dot...
     q1_dot q2_dot q3_dot I0 I1 I2 I3 x0 y0 x_dot phi fx fy t0 t1 t2 t3

% Define position vectors to each center of mass
r0                      = [ x0
                            y0 ];
                        
r1                      = [ x0 + (b0)*cos(phi + q0) + (a1)*...
                                    cos(pi/2 + q0 + q1)
                            y0 + (b0)*sin(phi + q0) + (a1)*...
                                    sin(pi/2 + q0 + q1) ];

r2                      = [ x0 + (b0)*cos(phi + q0) + (a1+b1)*...
                                cos(pi/2 + q0 + q1) + (a2)*...
                                cos(pi/2 + q0 + q1 + q2)
                            y0 + (b0)*sin(phi + q0) + (a1+b1)*...
                                sin(pi/2 + q0 + q1) + (a2)*...
                                sin(pi/2 + q0 + q1 + q2) ];

r3                      = [ x0 + (b0)*cos(phi + q0) + (a1+b1)*...
                                cos(pi/2 + q0 + q1) + (a2+b2)*...
                                cos(pi/2 + q0 + q1 + q2) + (a3)*...
                                cos(pi/2 + q0 + q1 + q2 + q3)
                            y0 + (b0)*sin(phi + q0) + (a1+b1)*...
                                sin(pi/2 + q0 + q1) + (a2+b2)*...
                                sin(pi/2 + q0 + q1 + q2) + (a3)*...
                                sin(pi/2 + q0 + q1 + q2 + q3)];

% Take the Jacobians
JM_r0                  = jacobian(r0,[x0,y0,q0,q1,q2,q3]);
JM_r1                  = jacobian(r1,[x0,y0,q0,q1,q2,q3]);
JM_r2                  = jacobian(r2,[x0,y0,q0,q1,q2,q3]);
JM_r3                  = jacobian(r3,[x0,y0,q0,q1,q2,q3]);

JM_r0_T                = transpose(JM_r0);
JM_r1_T                = transpose(JM_r1);
JM_r2_T                = transpose(JM_r2);
JM_r3_T                = transpose(JM_r3);

% Calculate linear kinetic energy
Trv                    = ((m0*JM_r0_T*JM_r0) + (m1*JM_r1_T*JM_r1) +...
                        (m2*JM_r2_T*JM_r2) + (m3*JM_r3_T*JM_r3));

wr0                    = [0; 0; q0];
wr1                    = [0; 0; q0+q1];
wr2                    = [0; 0; q0+q1+q2];
wr3                    = [0; 0; q0+q1+q2+q3];

JM_wr0                 = jacobian(wr0,[x0,y0,q0,q1,q2,q3]);
JM_wr1                 = jacobian(wr1,[x0,y0,q0,q1,q2,q3]);
JM_wr2                 = jacobian(wr2,[x0,y0,q0,q1,q2,q3]);
JM_wr3                 = jacobian(wr3,[x0,y0,q0,q1,q2,q3]);

JM_wr0_T               = transpose(JM_wr0);
JM_wr1_T               = transpose(JM_wr1);
JM_wr2_T               = transpose(JM_wr2);
JM_wr3_T               = transpose(JM_wr3);

% Calculate angular kinetic energy
Trw                    = ((I0*JM_wr0_T*JM_wr0) + (I1*JM_wr1_T*JM_wr1) +...
                        (I2*JM_wr2_T*JM_wr2) + (I3*JM_wr3_T*JM_wr3));

% Calculate inertia matrix
H                      = (Trv + Trw);
q                      = [x0,y0,q0,q1,q2,q3];
qdot                   = [x0_dot, y0_dot, q0_dot, q1_dot, q2_dot, q3_dot];

% Calculate coriolis matrix
for k = 1:6
    for i = 1:6
        for j = 1:6
            
            C(i,j,k) = 0.5*(diff(H(k,j),q(i)) + diff(H(k,i),q(j))...
                - diff(H(i,j),q(k)));
            
        end
    end
end
        
for k = 1:6
    for j = 1:6
             
            c(j,k) = C(1,j,k)*qdot(1) + C(2,j,k)*qdot(2) +...
                C(3,j,k)*qdot(3) + C(4,j,k)*qdot(4) + ...
                C(5,j,k)*qdot(5) + C(6,j,k)*qdot(6);
            
    end
end

Coriolis               = transpose(c);
Inertia                = H;

% Export functions
fprintf('Deriving Inertia...\n');
InertiaS = collect(simplify(Inertia),{'sin' 'cos'});
InertiaFunc = matlabFunction(InertiaS,'File','InertiaFunc3LINK');

fprintf('Deriving Coriolis...\n');
CoriolisS = collect(simplify(Coriolis),{'sin' 'cos'});
CoriolisFunc = matlabFunction(CoriolisS,'File','CoriolisFunc3LINK');



