%Optimizing the response:
%The following SIM will make use of UncoSat_initializa.m to optimize the
%SAC formulation for the LAB.

% Run_Initializer; %Run the sattelite initializer at least once.

%WHEN YOU RUN THIS SCRIPT, IT ASSUMES THAT CERTAIN VARIABLES, NAMELY
%ModelError AND ThrustOut ALREADY EXIST IN MEMORY. IF THEY DON'T DO SO BY
%RUNNING THE BASE SIMULATION ONCE!

Run_Initializer;%Runs the script that loads parameters:
% SimpleSAC

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


%PSO evolution Loop:
%limits
GAMMA_max(1) = 0e1;
GAMMA_max(2) = 0e1;
GAMMA_max(3) = 0e1;
GAMMA_max(4) = 0e1;
GAMMA_max(5) = 1e4;
GAMMA_max(6) = 1e4;
GAMMA_max(7) = 1e4;
GAMMA_max(8) = 1e4;
GAMMA_max(9) = 1;
GAMMA_max(10) = 1;
GAMMA_max(11) = 1;


GAMMA_min(1) = 0;
GAMMA_min(2) = 0;
GAMMA_min(3) = 0;
GAMMA_min(4) = 0;
GAMMA_min(5) = 0;
GAMMA_min(6) = 0;
GAMMA_min(7) = 0;
GAMMA_min(8) = 0;
GAMMA_min(9) = 0;
GAMMA_min(10) = 0;
GAMMA_min(11) = 0;


%Settings I change constantly:
if ~exist('Init','Var')
    PopSize = 20;%Number of agents in the population
    maxiter = 20;%Maximum iterations
    q = 1000000.*eye(3);
    r = eye(3);
else
    PopSize = Init.Agents;%Number of agents in the population
    maxiter = Init.Iter;%Maximum iterations
    q = Init.q;
    r = Init.r;
end
%Cost Function of the first run:
Cost_init = LQRCostFN(ModelError.Time, ModelError.Data, ThrustOut.Data, q,r)

halfPop = ceil(PopSize/2);
SaveVector = [ones(1,halfPop),zeros(1,PopSize-halfPop)];
GammaCount = 11; %Number of Gammas, should stay constant at e,x,u.
SimTimeInit = tsim;%initial simulation time.
% tsim = 100; %Time for cost simulation.

omega = 0.2;%How much of the previous velocity to keep (damp)
phi_p = 0.1;%Attraction to that particle's best position
phi_g = 1.0;%Attraction to overall best position

%Initial population:
for i = 1:PopSize
    for j = 1:GammaCount
        pos(i,j) = rand*(GAMMA_max(j)-GAMMA_min(j)) + GAMMA_min(j);%random starting position and velocity
        vel(i,j) = 0.5*(rand*2-1)*abs(GAMMA_max(j)-GAMMA_min(j));%Given by algorithm
        Best(i,j) = pos(i,j);%Best position in search space
    end
end
BestGamma = pos(1,:);%First agent
BestCost_tot = Cost_init;%First cost
BestParticle = 1;%Best particle index

%Force one value to work:
pos(1,:) = [GeP(1,1),GxP(1,1),GxP(4,4),GuP(1,1),GeI(1,1),GxI(1,1),GxI(4,4),GuI(1,1),sigma_eI,sigma_xI,sigma_uI];

%Run sim for initial population:
for i = 1:PopSize
    
    
% Initialize the learning rates for the adaptive controller:
GeP                            = pos(i,1).*eye(3);
GxP                            = blkdiag(pos(i,2).*eye(3),pos(i,3).*eye(3));
GuP                            = pos(i,4).*eye(3);
GeI                            = pos(i,5).*eye(3); 
GxI                            = blkdiag(pos(i,6).*eye(3),pos(i,7).*eye(3));
GuI                            = pos(i,8).*eye(3); 
% Set the forgetting rates. Higher values increase robustness to unknown
% disturbances:
sigma_eI                       = pos(i,9);
sigma_xI                       = pos(i,10);
sigma_uI                       = pos(i,11);    

Cost_X(i) = Cost_init*1e30;%Really big cost.
    try
        %For whatever reason, these lines are necessary to complete the
        %simulation.
        set_param(selectedFile(1:(end-4)),'SimulationMode','normal');
        assignin('base','simMode',1);
        options = simset('SrcWorkspace','base','DstWorkspace','base');
        sim(selectedFile(1:(end-4)),[],options)
        Cost_X(i) = LQRCostFN(ModelError.Time, ModelError.Data(:,1:2), ThrustOut.Data(:,1:2), q,r);
    catch E
        if isa(E, 'MSLException')
            Cost_X(i) = Cost_init*1e30;%Really big cost.
        end
    end
    
    if Cost_X(i) <= BestCost_tot
        BestGamma = pos(i,:);
        BestParticle = i;
        BestCost_tot = Cost_X(i);
    end
end
BestCost = Cost_X;%Initialize best cost for each particle
BestPos = pos;%Best Position
pos_TH = [];
%Updating and searching:
for iteration = 1:maxiter
    
    %figure out the order of the particle's current cost:
    
    %First sort the list:
    [SortedPositions,SortIndex] = sort(Cost_X);
    SaveList = SaveVector(SortIndex);%Boolean Vector with values to save.
    
    
    BestCost_TH(iteration) = BestCost_tot;
    
    for i = 1:PopSize %High cost members updated before low cost members are updated, since I'm not recording positions or velocities or costs...
        if ~SaveList(i)
            for j = GammaCount
                %Pick a random member of the top half of the population,
                %use his position for the next run. Repeat for velocity
                rp = rand;%Re-using rp and rg for the position and velocity selection procedure.
                rg = rand;
                
                pos(i,j) = pos(ceil(halfPop*rp),j);
                vel(i,j) = vel(ceil(halfPop*rg),j);
            end
        end
    end
    
    
    for i = 1:PopSize
        if SaveList(i) %If top member of population, update normally:
            for j = 1:GammaCount
                rp = rand;%Algorithm specifies this
                rg = rand;%alg
                
                vel(i,j) = omega*(vel(i,j)) + phi_p * rp * (BestPos(i,j) - pos(i,j)) + phi_g * rg * (BestPos(BestParticle,j) - pos(i,j));
                pos(i,j) = pos(i,j) + vel(i,j);%Velocity update
                if pos(i,j)>GAMMA_max(j)%Stop particles from leaving the search area
                    pos(i,j) = GAMMA_max(j);
                elseif pos(i,j)< GAMMA_min(j)
                    pos(i,j) = GAMMA_min(j);
                end
            end
        end
        
        
        %run sim with new value:
GeP                            = pos(i,1).*eye(3);
GxP                            = blkdiag(pos(i,2).*eye(3),pos(i,3).*eye(3));
GuP                            = pos(i,4).*eye(3);
GeI                            = pos(i,5).*eye(3); 
GxI                            = blkdiag(pos(i,6).*eye(3),pos(i,7).*eye(3));
GuI                            = pos(i,8).*eye(3); 
% Set the forgetting rates. Higher values increase robustness to unknown
% disturbances:
sigma_eI                       = pos(i,9);
sigma_xI                       = pos(i,10);
sigma_uI                       = pos(i,11);
        
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
        
        if Cost_X(i)< BestCost(i)
            BestCost(i) = Cost_X(i);
            if BestCost(i)<= BestCost_tot
                BestCost_tot = BestCost(i);
                BestPos = pos;
                BestParticle = i;
            end
        end
        
    end
    pos_TH(:,:,end+1) = pos;
    
    %display:
    besti = find(BestCost == min(BestCost));
    size_besti = size(besti);
    if size_besti(2) > 1
        besti = besti(1);
    end
    
    iteration
    BestCost_tot
    pos
    BestPos(besti,:)
    
    %     if (BestCost_tot<1)
    %         break
    %     end
    
    
    
end

%Find minimum cost:
GeP                            = pos(besti,1).*eye(3);
GxP                            = blkdiag(pos(besti,2).*eye(3),pos(besti,3).*eye(3));
GuP                            = pos(besti,4).*eye(3);
GeI                            = pos(besti,5).*eye(3); 
GxI                            = blkdiag(pos(besti,6).*eye(3),pos(besti,7).*eye(3));
GuI                            = pos(besti,8).*eye(3); 
% Set the forgetting rates. Higher values increase robustness to unknown
% disturbances:
sigma_eI                       = pos(besti,9);
sigma_xI                       = pos(besti,10);
sigma_uI                       = pos(besti,11);

BestPos(besti,:)

%Run the Optimized Sim:
% SimTime = SimTimeInit;
% sim(selectedFile(1:(end-4)))

%When both found a better controller than me:
%Q = identity, R = 10*identity
%0, 0, 104.2788, 9.5936, 36.9884, 30.6010
%Cost
%440.6349


fprintf('done \n')





