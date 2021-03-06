%This script will run all of our Particle swarms, and save their final
%results:

%WHEN YOU RUN THIS SCRIPT, IT ASSUMES THAT CERTAIN VARIABLES, NAMELY
%ModelError AND ThrustOut ALREADY EXIST IN MEMORY. IF THEY DON'T, DO SO BY
%RUNNING THE BASE SIMULATION ONCE!

Init.GeP = GeP;
Init.GxP = GxP;
Init.GuP = GuP;
Init.GeI = GeI;
Init.GxI = GxI;
Init.GuI = GuI;
Init.sigma_eI = sigma_eI;
Init.sigma_xI = sigma_xI;
Init.sigma_uI = sigma_uI;
Init.alpha    = alpha;
Init.tsim = tsim;

%Settings for ALL optimizations:
Init.Agents = 40;
Init.Iter = 20;
Init.q = 1.*eye(3);
Init.r = 10.*eye(3);

clearvars -except DE PSO SPSO SADE Init 

GeP = Init.GeP;     
GxP = Init.GxP;
GuP = Init.GuP;
GeI = Init.GeI;
GxI = Init.GxI;
GuI = Init.GuI;
sigma_eI = Init.sigma_eI;
sigma_xI = Init.sigma_xI;
sigma_uI = Init.sigma_uI;
alpha = Init.alpha
tsim = Init.tsim;

OptimizeSAC_DE

DE.GeP = GeP;
DE.GxP = GxP;
DE.GuP = GuP;
DE.GeI = GeI;
DE.GxI = GxI;
DE.GuI = GuI;
DE.sigma_eI = sigma_eI;
DE.sigma_xI = sigma_xI;
DE.sigma_uI = sigma_uI;
DE.alpha    = alpha;
DE.ModelError = ModelError;
DE.ThrustOut = ThrustOut;
DE.tsim = tsim;
DE.Cost = BestCost;

%And if you really care, you can just run the values you get to see if
%they're any good. This won't save much information from the runs... Sadly.

clearvars -except DE PSO SPSO SADE Init

GeP = Init.GeP;
GxP = Init.GxP;
GuP = Init.GuP;
GeI = Init.GeI;
GxI = Init.GxI;
GuI = Init.GuI;
sigma_eI = Init.sigma_eI;
sigma_xI = Init.sigma_xI;
sigma_uI = Init.sigma_uI;
alpha    = Init.alpha;
tsim = Init.tsim;

OptimizeSAC_PSO

PSO.GeP = GeP;
PSO.GxP = GxP;
PSO.GuP = GuP;
PSO.GeI = GeI;
PSO.GxI = GxI;
PSO.GuI = GuI;
PSO.sigma_eI = sigma_eI;
PSO.sigma_xI = sigma_xI;
PSO.sigma_uI = sigma_uI;
PSO.alpha    = alpha;
PSO.ModelError = ModelError;
PSO.ThrustOut = ThrustOut;
PSO.tsim = tsim;
PSO.Cost = BestCost_tot;

clearvars -except DE PSO SPSO SADE Init

GeP = Init.GeP;
GxP = Init.GxP;
GuP = Init.GuP;
GeI = Init.GeI;
GxI = Init.GxI;
GuI = Init.GuI;
sigma_eI = Init.sigma_eI;
sigma_xI = Init.sigma_xI;
sigma_uI = Init.sigma_uI;
alpha    = Init.alpha;
tsim = Init.tsim;

SPSO_OptimizationForSAC

SPSO.GeP = GeP;
SPSO.GxP = GxP;
SPSO.GuP = GuP;
SPSO.GeI = GeI;
SPSO.GxI = GxI;
SPSO.GuI = GuI;
SPSO.sigma_eI = sigma_eI;
SPSO.sigma_xI = sigma_xI;
SPSO.sigma_uI = sigma_uI;
SPSO.alpha    = alpha;
SPSO.tsim = tsim;
SPSO.Cost = BestCost_tot;

clearvars -except DE PSO SPSO SADE Init

GeP = Init.GeP;
GxP = Init.GxP;
GuP = Init.GuP;
GeI = Init.GeI;
GxI = Init.GxI;
GuI = Init.GuI;
sigma_eI = Init.sigma_eI;
sigma_xI = Init.sigma_xI;
sigma_uI = Init.sigma_uI;
alpha   = Init.alpha;
tsim = Init.tsim;

SADE_OptimizationForSAC

SADE.GeP = GeP;
SADE.GxP = GxP;
SADE.GuP = GuP;
SADE.GeI = GeI;
SADE.GxI = GxI;
SADE.GuI = GuI;
SADE.sigma_eI = sigma_eI;
SADE.sigma_xI = sigma_xI;
SADE.sigma_uI = sigma_uI;
SADE.alpha    = alpha;
SADE.ModelError = ModelError;
SADE.ThrustOut = ThrustOut;
SADE.tsim = tsim;
SADE.Cost = BestCost;

save('TestFFOptimizations400SecondsWOUTP.mat')

DE.GeP 
DE.GxP 
DE.GuP 
DE.GeI 
DE.GxI 
DE.GuI 
DE.sigma_eI 
DE.sigma_xI 
DE.sigma_uI 
DE.alpha
DE.Cost 

PSO.GeP 
PSO.GxP 
PSO.GuP 
PSO.GeI 
PSO.GxI 
PSO.GuI 
PSO.sigma_eI 
PSO.sigma_xI 
PSO.sigma_uI 
PSO.alpha
PSO.Cost 

SPSO.GeP 
SPSO.GxP 
SPSO.GuP 
SPSO.GeI 
SPSO.GxI 
SPSO.GuI 
SPSO.sigma_eI 
SPSO.sigma_xI 
SPSO.sigma_uI 
SPSO.alpha
SPSO.Cost 

SADE.GeP 
SADE.GxP 
SADE.GuP 
SADE.GeI 
SADE.GxI 
SADE.GuI 
SADE.sigma_eI 
SADE.sigma_xI 
SADE.sigma_uI
SADE.alpha
SADE.Cost 
















