function [GainMat,Error] = LinearSACConverges(Plant,Model)
%LinearSACConverges Checks if SAC Converges on Model
%   Given a linear plant and model, can the SAC converge on constant gains?
%   If there is an ouput, the answer is yes. Converged gain values are
%   given. These may not be reached in practice.
%Input:
%   Plant - Structure with linear plant model. Of form Plant.A, Plant.B,
%       Plant.C, Plant.D, for state space system x_dot = A*x+B*u, y = C*x + D*u
%       Note that the number of inputs must equal the number of outputs.
%   Model - Structure with linear reference model. Of form Model.A,
%       Model.B, Model.C, Model.D, for the same state space formulation.
%Ouputs:
%   GainMatrix - Output gain matrix, of the form: [S11, S12;KxStar, KuStar]
%   Error  - Error of the equality condition required.

Ap = Plant.A;
Bp = Plant.B;
Cp = Plant.C;
Dp = Plant.D;

Am = Model.A;
Bm = Model.B;
Cm = Model.C;
Dm = Model.D;


SizeA = size(Ap);%Should be square
SizeB = size(Bp);
SizeC = size(Cp);
SizeD = size(Dp);


M = [Ap,Bp;Cp,Dp];
N = inv(M);
%Solving the matrix equality:
N11 = N(1:SizeA(1),1:SizeA(2));
N12 = N(1:SizeA(2),SizeA(2)+1:end);
N21 = N(SizeA(1)+1:end,1:SizeA(1));
N22 = N(SizeA(1)+1:end,SizeA(2)+1:end);

%Lyapunov equation for S11:
a = N11;
b = -inv(Am);
c = N12*Cm*inv(Am);
S11 = lyap(a,b,c);
S12 = N11*S11*Bm;
KxStar = N21*S11*Am;
KuStar = N21*S11*Bm;

GainMat = [S11,S12;KxStar,KuStar];

%Then check again:
Error = M*GainMat - [S11*Model.A,S11*Model.B;Model.C,Model.D];


end

