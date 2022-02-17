function [A_cone, b_cone] = return_square_cone(ha, d, C_CB)
%% DESCRIPTION:
% returns a square cone approximation.

%% INPUTS:
% ha - the half angle of the square cone
% d_B - the origin of the square cone in the body frame

%% OUTPUTS:
% A_cone - a matrix describing the "cone" in the body-fixed frame.
% b_cone - the cone inequality matrix:

% v_B^T * (r_B - d_B) >= 0
% v_B^T * r_B       >= v_B^T * d_B
% -v_C^T * C_CB * r_B <= -v_C^T * C_CB * d_B

% A * C_BI* r_I <= A * d_B

% Give four unit vectors (one for each side of the cone)
py = [0;1;0];
my = -py;
pz = [0;0;1];
mz = -pz;

% Now, rotate these vectors by the correct half angles:
py = R3(-ha) * py;
my = R3(ha) * my;
pz = R2(ha) * pz;
mz = R2(-ha) * mz;

% Now we can give the description of the cone as seen from the body frame:
% Note, this also needs to be rotated into the body-frame, as that is where
% the measurements will take place.
A_cone = -[py' ; 
           my' ; 
           pz' ; 
           mz'] * C_CB;

b_cone = A_cone * d;

end

