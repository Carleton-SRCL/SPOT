%Set gains:

if strcmp('Hand',Gains)
% Initialize the learning rates for the adaptive controller:
%Testing values:
% GeP                            = 100*eye(3,3);
% GxP                            = blkdiag(6e3.*eye(3),0.*eye(3));
% GuP                            = 0*eye(3,3);
% GeI                            = 1e0*eye(3,3); 
% GxI                            = blkdiag(0.*eye(3),1e6.*eye(3));
% GuI                            = 0*eye(3,3); 

%Values that work WELL:
GeP                            = 100*eye(3,3);
GxP                            = 1e5*eye(6,6);
GuP                            = 100*eye(3,3);
GeI                            = 1e5*eye(3,3); 
GxI                            = 1e1*eye(6,6); 
GuI                            = 1e1*eye(3,3); 

%For feedforward parallelized version:
GeP                            = 1e5*eye(3,3);
GxP                            = 1e2*eye(6,6);
GuP                            = 1e1*eye(3,3);
GeI                            = 1e3*eye(3,3); 
GxI                            = 1e1*eye(6,6); 
GuI                            = 1e1*eye(3,3);

%Tweaking feedforward parallelized version, For Disturbance Testing:
%First without gains:
GeP                            = 0e0*eye(3,3);  
GxP                            = 0e1*eye(6,6);
GuP                            = 0e1*eye(3,3);
GeI                            = 1e4*eye(3,3); 
GxI                            = 1e-1*eye(6,6); 
GuI                            = 1e-2*eye(3,3);

%SaDE Memory:
% GeP                            = 7.7738e5*eye(3,3);
% GxP                            = 5.6694e5*eye(6,6);
% GuP                            = 0*eye(3,3);
% GeI                            = 3.7777e5*eye(3,3); 
% GxI                            = blkdiag(zeros(3),1.5861e5.*eye(3,3)); 
% GuI                            = 0*eye(3,3); 

%Limits:
% GeI = 1e6

% Set the forgetting rates. Higher values increase robustness to unknown
% disturbances:
sigma_eI                       = 0.001;
sigma_xI                       = 0;
sigma_uI                       = 0;

alpha                          = 0;%Screw it, break any scripts with an alpha in there.

%Gains that the nonlinear optimization gives (Somehow work now?):
GeP                            = 12.7794*eye(3,3);
GxP                            = blkdiag(1.8452e-5*eye(3),160.2038*eye(3));
GuP                            = 3.2479*eye(3,3);
GeI                            = 6.6287*eye(3,3); 
GxI                            = blkdiag(4.7686e-5*eye(3),160.2038*eye(3)); 
GuI                            = 2.6344e-4*eye(3,3);

sigme_eI = 0.003;
xsigmas = [0.0033,0;0,0.0261];
sigma_xI = blkdiag(xsigmas,xsigmas,xsigmas)';
sigma_uI = 0.0002;

else
    
% Initialize the learning rates for the adaptive controller:
GeP                            = Glow.(Gains).GeP;
GxP                            = Glow.(Gains).GxP;
GuP                            = Glow.(Gains).GuP;
GeI                            = Glow.(Gains).GeI; 
GxI                            = Glow.(Gains).GxI; 
GuI                            = Glow.(Gains).GuI; 

% Set the forgetting rates. Higher values increase robustness to unknown
% disturbances:
sigma_eI                       = Glow.(Gains).sigma_eI;
sigma_xI                       = Glow.(Gains).sigma_xI;
sigma_uI                       = Glow.(Gains).sigma_uI;

if exist(['Glow.',Gains,'.alpha'])
alpha                          = Glow.(Gains).alpha;
else
   alpha = 1; 
end
    
end