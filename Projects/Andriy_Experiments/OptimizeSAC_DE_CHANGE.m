%Optimizing the response:
%The following SIM will make use of UncoSat_initializa.m to optimize the
%SAC formulation for the LAB.

% UncoSat_initializa; %Run the sattelite initializer at least once.
%puts relevant variables in memory.

%WHEN YOU RUN THIS SCRIPT, IT ASSUMES THAT CERTAIN VARIABLES, NAMELY
%ModelError AND ThrustOut ALREADY EXIST IN MEMORY. IF THEY DON'T DO SO BY
%RUNNING THE BASE SIMULATION ONCE!

Run_Initializer;%Runs the script that loads parameters:

selectedFile = 'Predmyrskyy_SwarmOptimizedSAC.slx';

R = 1;%Set the platfoprm selection:
A = 0;
B = 0;
D = 0;
platformSelection = 1;%The index for the platform selection.

%Then run the simulation once to load the "Best Solution" that's in the
%Run_Initializer script:
%For whatever reason, these lines are necessary to complete the
%simulation.
set_param(selectedFile(1:(end-4)),'SimulationMode','normal');
assignin('base','simMode',1);
options = simset('SrcWorkspace','base','DstWorkspace','base');
sim(selectedFile(1:(end-4)),[],options)

%Cost Function of the first run:
q = 1000.*eye(3);
r = eye(3);
Cost_init = LQRCostFN(ModelError.Time, ModelError.Data, ThrustOut.Data, q,r)
%NOTE: Cost_init uses the cost of the original sim time, while the
%optimization script uses the SimTime defined below. If SimTime below is
%longer than the initialization SimTime, Error may appear, due to the way
%unstable gains are set to Cost_init. ALWAYS USE SimTime in optimization <
%SimTime in initialization.

%Differential evolution Loop:
%limits
GAMMAe_max = 100;
GAMMAu_max = 100;
GAMMAx_max = 100;
SIGMA_max = 100;

GAMMAe_min = 0;
GAMMAu_min = 0;
GAMMAx_min = 0;
SIGMA_min = 0;

a = 1;%"Slope" variable for sigmoid function used.

GAMMAe_sig = @(x) GAMMAe_max/(1 + exp(-a * x)) + GAMMAe_min;
GAMMAu_sig = @(x) GAMMAu_max/(1 + exp(-a * x)) + GAMMAu_min;
GAMMAx_sig = @(x) GAMMAx_max/(1 + exp(-a * x)) + GAMMAx_min;
SIGMA_sig = @(x) SIGMA_max/(1 + exp(-a * x)) + SIGMA_min;

PopSize = 100;%Number of agents in the population
GammaCount = 9; %Number of Gammas, should stay constant at e,x,u.
maxiter = 100;%Maximum iterations
SimTimeInit = tsim;
% SimTime = 400; %Time for cost simulation.
ScalingFactor = 1;
CrossoverRate = 0.5;


%Initial population:
% InitialPop = rand(max([PopSize, GammaCount]));
% for i = 1:GammaCount
%     for j = 1:PopSize
%         X(i,j) = InitialPop(i,j);
%     end
% end
% %Maps 0-1 onto -inf to inf:
% for i = 1:GammaCount
%     X(i,:) = log(X(i,:)) - log(1-X(i,:));%Limited in practice...
% end
% %Find the best from the population:
%
% %Force one of the gammas to be the initial gammas, making not nonsensical
% %gains.
% %inverse of sigmoid:
% GAMMAe_sig_inv = @(y)  log(GAMMAe_max/(y - GAMMAe_min) - 1)/-a;
% GAMMAu_sig_inv = @(y)  log(GAMMAu_max/(y - GAMMAu_min) - 1)/-a;
% GAMMAx_sig_inv = @(y)  log(GAMMAx_max/(y - GAMMAx_min) - 1)/-a;
% SIGMA_sig_inv = @(y)  log(SIGMA_max/(y - SIGMA_min) - 1)/-a;

%Redo initial population assignment, because for whatever reason (Double
%precision) The inverse sigmoid thing isn't working too great:

%Initial population:
InitialPop = rand(max([PopSize, GammaCount]));
for i = 1:GammaCount
    for j = 1:PopSize
        X(i,j) = InitialPop(i,j);
    end
    
    %For each of the positions, put it into the right range:
    switch i
        
        case 1
            X(i,:) = X(i,:).*GAMMAe_max;
        case 2
            X(i,:) = X(i,:).*GAMMAe_max;
        case 3
            X(i,:) = X(i,:).*GAMMAu_max;
        case 4
            X(i,:) = X(i,:).*GAMMAu_max;
        case 5
            X(i,:) = X(i,:).*GAMMAx_max;
        case 6
            X(i,:) = X(i,:).*GAMMAx_max;
        case 7
            X(i,:) = X(i,:).*SIGMA_max;
        case 8
            X(i,:) = X(i,:).*SIGMA_max;
        case 9
            X(i,:) = X(i,:).*SIGMA_max;
    end
    
end

%Initial population is now within bounds


%Initial Population, ensures at least one agent that doesn't diverge.
X(:,1) = [GeP(1,1);GeI(1,1); ...
    GuP(1,1); GuI(1,1); ...
    GxP(1,1); GxP(1,1);sigma_eI;...
    sigma_uI;sigma_xI];

GAMMAx = zeros(4,2);

%Determine cost of initial population:
for i = 1:PopSize
    
    %Assign population values
    GeP = X(1,i).*eye(3);
    GeI = X(2,i).*eye(3);
    GuP = X(3,i).*eye(3);
    GuI = X(4,i).*eye(3);
    GxP = X(5,i).*eye(6);
    GxI = X(6,i).*eye(6);
    sigma_eI = X(7,i);
    sigma_uI = X(8,i);
    sigma_xI = X(9,i);
    
    try
        %For whatever reason, these lines are necessary to complete the
        %simulation.
        set_param(selectedFile(1:(end-4)),'SimulationMode','normal');
        assignin('base','simMode',1);
        options = simset('SrcWorkspace','base','DstWorkspace','base');
        sim(selectedFile(1:(end-4)),[],options)
        Cost_X(i) = LQRCostFN(ModelError.Time, ModelError.Data, ThrustOut.Data, q,r);
    catch E
        if isa(E, 'MSLException')
            Cost_X(i) = Cost_init*1e30;%Really big cost.
        end
    end
    
    
end

Best_i = find(Cost_X == min(Cost_X));%Find best cost.
M = zeros(GammaCount,PopSize);

for n = 1:maxiter
    X_base = X(:,Best_i);
    
    X_size = size(X_base);
    if X_size(2) > 1
        X_base = X(:,1);%In the case where there is no best.
    end
    
    for i = 1:PopSize
        %Created Mutated Vector:
        
        randI = [1,1];
        while (randI(1) == randI(2)| randI(1) == Best_i | randI(2) == Best_i)
            randI = randi([1,PopSize],1,2); % Keep coming up with random integers
        end                                % until it works
        
        %Mutated Vector:
        M(:,i) = X_base + ScalingFactor.*(X(:,randI(1)) - X(:, randI(2)));
        
        %For each element of the mutated vector, ensure it is within
        %bounds:
        
        %There's probably a more compact and faster way of doing this,
        %Not doing that here to save (my personal) time.
        for k = 1:GammaCount
            switch k
                case 1
                    if M(k,i) >= GAMMAe_max
                        M(k,i) = GAMMAe_max;
                    elseif M(k,i) <= GAMMAe_min
                        M(k,i) = GAMMAe_min;
                    end
                case 2
                    if M(k,i) >= GAMMAe_max
                        M(k,i) = GAMMAe_max;
                    elseif M(k,i) <= GAMMAe_min
                        M(k,i) = GAMMAe_min;
                    end
                case 3
                    if M(k,i) >= GAMMAu_max
                        M(k,i) = GAMMAu_max;
                    elseif M(k,i) <= GAMMAu_min
                        M(k,i) = GAMMAu_min;
                    end
                case 4
                    if M(k,i) >= GAMMAu_max
                        M(k,i) = GAMMAu_max;
                    elseif M(k,i) <= GAMMAu_min
                        M(k,i) = GAMMAu_min;
                    end
                case 5
                    if M(k,i) >= GAMMAx_max
                        M(k,i) = GAMMAx_max;
                    elseif M(k,i) <= GAMMAx_min
                        M(k,i) = GAMMAx_min;
                    end
                case 6
                    if M(k,i) >= GAMMAx_max
                        M(k,i) = GAMMAx_max;
                    elseif M(k,i) <= GAMMAx_min
                        M(k,i) = GAMMAx_min;
                    end
                case 7
                    if M(k,i) >= SIGMA_max
                        M(k,i) = SIGMA_max;
                    elseif M(k,i) <= SIGMA_min
                        M(k,i) = SIGMA_min;
                    end
                case 8
                    if M(k,i) >= SIGMA_max
                        M(k,i) = SIGMA_max;
                    elseif M(k,i) <= SIGMA_min
                        M(k,i) = SIGMA_min;
                    end
                case 9
                    if M(k,i) >= SIGMA_max
                        M(k,i) = SIGMA_max;
                    elseif M(k,i) <= SIGMA_min
                        M(k,i) = SIGMA_min;
                    end
            end
            
        end
        
        %Target Vector:
        jr = randi([0,GammaCount],1,1);
        for j = 1:GammaCount
            rand01 = rand;
            if (rand01 <= CrossoverRate ||  j == jr)
                U(j,i) = M(j,i);%Mutate
            else
                U(j,i) = X(j,i);%Keep the same
            end
        end
    end
    
    %Now with target vectors made for each member of the population
    %We run our cost test again:
    for i = 1:PopSize
        
        %Assign population values
        GeP = U(1,i).*eye(3);
        GeI = U(2,i).*eye(3);
        GuP = U(3,i).*eye(3);
        GuI = U(4,i).*eye(3);
        GxP = U(5,i).*eye(6);
        GxI = U(6,i).*eye(6);
        sigma_eI = U(7,i);
        sigma_uI = U(8,i);
        sigma_xI = U(9,i);
        
        try
            %For whatever reason, these lines are necessary to complete the
            %simulation.
            set_param(selectedFile(1:(end-4)),'SimulationMode','normal');
            assignin('base','simMode',1);
            options = simset('SrcWorkspace','base','DstWorkspace','base');
            sim(selectedFile(1:(end-4)),[],options)
            Cost_U(i) = LQRCostFN(ModelError.Time, ModelError.Data, ThrustOut.Data, q,r);
        catch E
            if isa(E, 'MSLException')
                Cost_U(i) = Cost_init*1e30;%Really big cost.
            end
        end
        
        if Cost_U(i) < Cost_X(i)
            X(:,i) = U(:,i);
            Cost_X(i) = Cost_U(i);
        end
    end
    
    Best_i = find(Cost_X == min(Cost_X));%Find best cost.
    
    Best_i_size = size(Best_i);
    if Best_i_size(2) > 1
        Best_i = Best_i(:,1);%In the case where there is no best.
    end
    
    %display:
    iteration = n
    CurrentCost = Cost_X(Best_i)
    CurrentParameters = X(:,Best_i)
    
    X
%     U
%     Cost_U
    
    
    
    %     if (min(Cost)<1)%arbitrary breakpoint
    %         break
    %     end
    
end



Best_i = find(Cost_X == min(Cost_X));%Find best cost.
sizeBest_i = size(Best_i);

if sizeBest_i(2) >1
    Best_i = Best_i(1);
end

BestCost = Cost_X(Best_i)
BestParameters = X(:,Best_i)

%Assign population values
GeP = X(1,Best_i).*eye(3);
GeI = X(2,Best_i).*eye(3);
GuP = X(3,Best_i).*eye(3);
GuI = X(4,Best_i).*eye(3);
GxP = X(5,Best_i).*eye(6);
GxI = X(6,Best_i).*eye(6);
sigma_eI = X(7,Best_i);
sigma_uI = X(8,Best_i);
sigma_xI = X(9,Best_i);



%When both were better than me
%Q = I, R = 10*I
%0, 0, 110.5470, 0.1995, 109.5065, 104.2436
%Cost
%440.6351

%Run the Optimized Sim:
% SimTime = SimTimeInit;
% sim(SimName)

fprintf('done \n')




