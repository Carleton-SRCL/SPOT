%Making figures for the optimizations that we made:
close all
clear all

%makes sure we have the relevant variables available:
Run_Initializer
selectedFile = 'Predmyrskyy_SwarmOptimizedSAC.slx';
R = 1;%Set the platfoprm selection:
A = 0;
B = 0;
D = 0;
platformSelection = 1;%The index for the platform selection.
load("plots.mat",'LQR')%Loads some old values for comparison
load('4040Optimizations200Seconds.mat','DE','SADE','PSO','SPSO');

PosCmd.Time = Path_xyz.time;
PosCmd.Data = Path_xyz.signals.values;
PosComAt = @(t,i) interp1(PosCmd.Time,PosCmd.Data(:,i),t);
%Run the DE sim and plot relevant stuff:
Gains = 'DE';
SetGains

%For whatever reason, these lines are necessary to complete the
%simulation.
set_param(selectedFile(1:(end-4)),'SimulationMode','normal');
assignin('base','simMode',1);
options = simset('SrcWorkspace','base','DstWorkspace','base');
sim(selectedFile(1:(end-4)),[],options)

ChaserPos.Data = zeros(length(TargetPos.Data),2);
ChaserPos.Time = TargetPos.Time;

DE.Traj.Data = [ChaserPos.Data(:,1)-TargetPos.Data(:,1),ChaserPos.Data(:,2)-TargetPos.Data(:,2)];
DE.Traj.Time = ChaserPos.Time;
DE.Error.Time = ModelError.Time;
DE.Error.Data = ModelError.Data;
DE.ThrustOut = ThrustOut;

figure(12)
plot(TargetPos.Time, TargetPos.Data)
grid on
ylabel('Position (m)')
xlabel('time (s)')
legend('x axis position','y axis position','z axis position')
title('PID SAC Position Response')

%errors:
figure(13)
plot(Error.Time, Error.Data)
grid on
ylabel('Position error (m)')
xlabel('time (s)')
legend('x axis position','y axis position','z axis position')
title('PID SAC Position Error')

%Modelling:
figure(14)
plot(ModelError.Time, ModelError.Data)
grid on
ylabel('Position error (m)')
xlabel('time (s)')
legend('x axis position','y axis position','z axis position')
title('Designed SAC Model Error')

figure(15)
plot(ModelResp.Time,ModelResp.Data)
grid on
ylabel('Position (m)')
xlabel('time (s)')
legend('x axis position','y axis position','z axis position')
title('Reference Model Response')

figure(16)
plot(PosCmd.Time,PosCmd.Data)
grid on
ylabel('Position Command (m)')
xlabel('time (s)')
legend('x axis position command','y axis position command','z axis position command')
title('Position Command')

%Cycloid figure:
figure(17)
plot(track(:,1),track(:,2))

%relative positions:
figure(18)
plot(ChaserPos.Data(:,1)-TargetPos.Data(:,1),ChaserPos.Data(:,2)-TargetPos.Data(:,2))


%Run the PSO sim and plot relevant stuff:
Gains = 'PSO';
SetGains

%For whatever reason, these lines are necessary to complete the
%simulation.
set_param(selectedFile(1:(end-4)),'SimulationMode','normal');
assignin('base','simMode',1);
options = simset('SrcWorkspace','base','DstWorkspace','base');
sim(selectedFile(1:(end-4)),[],options)

PSO.Traj.Data = [ChaserPos.Data(:,1)-TargetPos.Data(:,1),ChaserPos.Data(:,2)-TargetPos.Data(:,2)];
PSO.Traj.Time = ChaserPos.Time;
PSO.Error.Time = ModelError.Time;
PSO.Error.Data = ModelError.Data;
PSO.ThrustOut = ThrustOut;

figure(22)
plot(ChaserPos.Time, ChaserPos.Data)
grid on
ylabel('Position (m)')
xlabel('time (s)')
legend('x axis position','y axis position','z axis position')
title('PID SAC Position Response')

%errors:
figure(23)
plot(Error.Time, Error.Data)
grid on
ylabel('Position error (m)')
xlabel('time (s)')
legend('x axis position','y axis position','z axis position')
title('PID SAC Position Error')

%Modelling:
figure(24)
plot(ModelError.Time, ModelError.Data)
grid on
ylabel('Position error (m)')
xlabel('time (s)')
legend('x axis position','y axis position','z axis position')
title('Designed SAC Model Error')

figure(25)
plot(ModelResp.Time,ModelResp.Data)
grid on
ylabel('Position (m)')
xlabel('time (s)')
legend('x axis position','y axis position','z axis position')
title('Reference Model Response')

figure(26)
plot(PosCmd.Time,PosCmd.Data)
grid on
ylabel('Position Command (m)')
xlabel('time (s)')
legend('x axis position command','y axis position command','z axis position command')
title('Position Command')

%Cycloid figure:
figure(27)
plot(track(:,1),track(:,2))

%relative positions:
figure(28)
plot(ChaserPos.Data(:,1)-TargetPos.Data(:,1),ChaserPos.Data(:,2)-TargetPos.Data(:,2))

%Run the SPSO sim and plot relevant stuff:
Gains = 'SPSO';
SetGains

%For whatever reason, these lines are necessary to complete the
%simulation.
set_param(selectedFile(1:(end-4)),'SimulationMode','normal');
assignin('base','simMode',1);
options = simset('SrcWorkspace','base','DstWorkspace','base');
sim(selectedFile(1:(end-4)),[],options)

SPSO.Traj.Data = [ChaserPos.Data(:,1)-TargetPos.Data(:,1),ChaserPos.Data(:,2)-TargetPos.Data(:,2)];
SPSO.Traj.Time = ChaserPos.Time;
SPSO.Error.Time = ModelError.Time;
SPSO.Error.Data = ModelError.Data;
SPSO.ThrustOut = ThrustOut;

figure(32)
plot(ChaserPos.Time, ChaserPos.Data)
grid on
ylabel('Position (m)')
xlabel('time (s)')
legend('x axis position','y axis position','z axis position')
title('PID SAC Position Response')

%errors:
figure(33)
plot(Error.Time, Error.Data)
grid on
ylabel('Position error (m)')
xlabel('time (s)')
legend('x axis position','y axis position','z axis position')
title('PID SAC Position Error')

%Modelling:
figure(34)
plot(ModelError.Time, ModelError.Data)
grid on
ylabel('Position error (m)')
xlabel('time (s)')
legend('x axis position','y axis position','z axis position')
title('Designed SAC Model Error')

figure(35)
plot(ModelResp.Time,ModelResp.Data)
grid on
ylabel('Position (m)')
xlabel('time (s)')
legend('x axis position','y axis position','z axis position')
title('Reference Model Response')

figure(36)
plot(PosCmd.Time,PosCmd.Data)
grid on
ylabel('Position Command (m)')
xlabel('time (s)')
legend('x axis position command','y axis position command','z axis position command')
title('Position Command')

%Cycloid figure:
figure(37)
plot(track(:,1),track(:,2))

%relative positions:
figure(38)
plot(ChaserPos.Data(:,1)-TargetPos.Data(:,1),ChaserPos.Data(:,2)-TargetPos.Data(:,2))


%Run the PSO sim and plot relevant stuff:
Gains = 'SADE';
SetGains

%For whatever reason, these lines are necessary to complete the
%simulation.
set_param(selectedFile(1:(end-4)),'SimulationMode','normal');
assignin('base','simMode',1);
options = simset('SrcWorkspace','base','DstWorkspace','base');
sim(selectedFile(1:(end-4)),[],options)

SADE.Traj.Data = [ChaserPos.Data(:,1)-TargetPos.Data(:,1),ChaserPos.Data(:,2)-TargetPos.Data(:,2)];
SADE.Traj.Time = ChaserPos.Time;
SADE.Error.Time = ModelError.Time;
SADE.Error.Data = ModelError.Data;
SADE.ThrustOut = ThrustOut;

figure(42)
plot(ChaserPos.Time, ChaserPos.Data)
grid on
ylabel('Position (m)')
xlabel('time (s)')
legend('x axis position','y axis position','z axis position')
title('PID SAC Position Response')

%errors:
figure(43)
plot(Error.Time, Error.Data)
grid on
ylabel('Position error (m)')
xlabel('time (s)')
legend('x axis position','y axis position','z axis position')
title('PID SAC Position Error')

%Modelling:
figure(44)
plot(ModelError.Time, ModelError.Data)
grid on
ylabel('Position error (m)')
xlabel('time (s)')
legend('x axis position','y axis position','z axis position')
title('Designed SAC Model Error')

figure(45)
plot(ModelResp.Time,ModelResp.Data)
grid on
ylabel('Position (m)')
xlabel('time (s)')
legend('x axis position','y axis position','z axis position')
title('Reference Model Response')

figure(46)
plot(PosCmd.Time,PosCmd.Data)
grid on
ylabel('Position Command (m)')
xlabel('time (s)')
legend('x axis position command','y axis position command','z axis position command')
title('Position Command')

%Cycloid figure:
figure(47)
plot(track(:,1),track(:,2))

%relative positions:
figure(48)
plot(ChaserPos.Data(:,1)-TargetPos.Data(:,1),ChaserPos.Data(:,2)-TargetPos.Data(:,2))

%Run the Handmade SAC sim and plot relevant stuff:
Gains = 'Hand';
SetGains

%For whatever reason, these lines are necessary to complete the
%simulation.
set_param(selectedFile(1:(end-4)),'SimulationMode','normal');
assignin('base','simMode',1);
options = simset('SrcWorkspace','base','DstWorkspace','base');
sim(selectedFile(1:(end-4)),[],options)

Hand.Traj.Data = [ChaserPos.Data(:,1)-TargetPos.Data(:,1),ChaserPos.Data(:,2)-TargetPos.Data(:,2)];
Hand.Traj.Time = ChaserPos.Time;
Hand.Error.Time = ModelError.Time;
Hand.Error.Data = ModelError.Data;
Hand.ThrustOut = ThrustOut;

%Loading LQE controller values:

% load('LQRPath.mat')
LQR.Traj.Data = [LQR.TargetPos.Data(:,1),LQR.TargetPos.Data(:,2)];
LQR.Traj.Time = LQR.TargetPos.Time;
LQR.Error.Time = LQR.Traj.Time;
LQR.Error.Data = LQR.Traj.Data - [PosComAt(LQR.Traj.Time,1),PosComAt(LQR.Traj.Time,2)];


%% MAKE COMPOSITE PLOTS:

figure
%Hand made and LQR:
subplot(3,2,1)
pdfprep
%Trajectories with time
plot(ModelResp.Data(:,1)-TargetPos.Data(:,1),ModelResp.Data(:,2)-TargetPos.Data(:,2))
grid on
hold on
plot(Hand.Traj.Data(:,1),Hand.Traj.Data(:,2))
plot(LQR.Traj.Data(:,1),LQR.Traj.Data(:,2))
legend('Model','Designed SAC','LQR','Location','northwest')
xlabel('x position (m)')
ylabel('y position (m)')


subplot(3,2,2)
pdfprep
%Errors with time:
plot(Hand.Error.Time,sqrt(sum(Hand.Error.Data.^2,2)))
grid on
hold on
plot(LQR.Error.Time,sqrt(sum(LQR.Error.Data.^2,2)))
legend('Manually Designed SAC','LQR')
xlabel('Time (s)')
ylabel('Position Error (m)')


%DE and SADE
subplot(3,2,3)
pdfprep
%Trajectories with time
plot(ModelResp.Data(:,1)-TargetPos.Data(:,1),ModelResp.Data(:,2)-TargetPos.Data(:,2))
grid on
hold on
plot(DE.Traj.Data(:,1),DE.Traj.Data(:,2))
plot(SADE.Traj.Data(:,1),SADE.Traj.Data(:,2))
legend('Model','DE','SaDE','Location','northwest')
xlabel('x position (m)')
ylabel('y position (m)')


subplot(3,2,4)
pdfprep
%Errors with time:
plot(DE.Error.Time,sqrt(sum(DE.Error.Data.^2,2)))
grid on
hold on
plot(SADE.Error.Time,sqrt(sum(SADE.Error.Data.^2,2)))
legend('DE','SaDE')
xlabel('Time (s)')
ylabel('Position Error (m)')


%PSO and SPSO:
subplot(3,2,5)
pdfprep
%Trajectories with time
plot(ModelResp.Data(:,1)-TargetPos.Data(:,1),ModelResp.Data(:,2)-TargetPos.Data(:,2))
grid on
hold on
plot(PSO.Traj.Data(:,1),PSO.Traj.Data(:,2))
plot(SPSO.Traj.Data(:,1),SPSO.Traj.Data(:,2))
legend('Model','PSO','SPSO','Location','northwest')
xlabel('x position (m)')
ylabel('y position (m)')


subplot(3,2,6)
%Errors with time:
plot(PSO.Error.Time,sqrt(sum(PSO.Error.Data.^2,2)))
grid on
hold on
plot(SPSO.Error.Time,sqrt(sum(SPSO.Error.Data.^2,2)))
legend('PSO','SPSO')
xlabel('Time (s)')
ylabel('Position Error (m)')
pdfplot('SwarmOptFig')


%% Singular Plots:
figure(100)
%Hand made and LQR:
%Trajectories with time
plot(ModelResp.Data(:,1)-TargetPos.Data(:,1),ModelResp.Data(:,2)-TargetPos.Data(:,2),'--k')
grid on
hold on
plot(Hand.Traj.Data(:,1),Hand.Traj.Data(:,2),'k')
plot(LQR.Traj.Data(:,1),LQR.Traj.Data(:,2),'r')
legend('Desired Trajectory','Designed SAC','LQR','Location','northeast')
xlabel('x position (m)')
ylabel('y position (m)')
pdfplot('SwarmOptFig1')


figure(101)
%Errors with time:
plot(Hand.Error.Time,sqrt(sum(Hand.Error.Data.^2,2)),'k')
grid on
hold on
plot(LQR.Error.Time,sqrt(sum(LQR.Error.Data.^2,2)),'r')
legend('SAC','LQR')
xlabel('Time (s)')
ylabel('Position Error (m)')
pdfplot('SwarmOptFig2')

%DE and SADE
figure(102)
%Trajectories with time
plot(ModelResp.Data(:,1)-TargetPos.Data(:,1),ModelResp.Data(:,2)-TargetPos.Data(:,2),'--k')
grid on
hold on
plot(DE.Traj.Data(:,1),DE.Traj.Data(:,2),'k')
plot(SADE.Traj.Data(:,1),SADE.Traj.Data(:,2),'r')
legend('Desired Trajectory','DE','SaDE','Location','northeast')
xlabel('x position (m)')
ylabel('y position (m)')
pdfplot('SwarmOptFig3')

figure(103)
%Errors with time:
plot(DE.Error.Time,sqrt(sum(DE.Error.Data.^2,2)),'k')
grid on
hold on
plot(SADE.Error.Time,sqrt(sum(SADE.Error.Data.^2,2)),'r')
legend('DE','SaDE')
xlabel('Time (s)')
ylabel('Position Error (m)')
pdfplot('SwarmOptFig4')


%PSO and SPSO:
figure(104)
%Trajectories with time
plot(ModelResp.Data(:,1)-TargetPos.Data(:,1),ModelResp.Data(:,2)-TargetPos.Data(:,2),'--k')
grid on
hold on
plot(PSO.Traj.Data(:,1),PSO.Traj.Data(:,2),'k')
plot(SPSO.Traj.Data(:,1),SPSO.Traj.Data(:,2),'r')
legend('Desired Trajectory','PSO','SPSO','Location','northeast')
xlabel('x position (m)')
ylabel('y position (m)')
pdfplot('SwarmOptFig5')


figure(105)
%Errors with time:
plot(PSO.Error.Time,sqrt(sum(PSO.Error.Data.^2,2)),'k')
grid on
hold on
plot(SPSO.Error.Time,sqrt(sum(SPSO.Error.Data.^2,2)),'r')
legend('PSO','SPSO')
xlabel('Time (s)')
ylabel('Position Error (m)')
pdfplot('SwarmOptFig6')


%But now forget all of that:
% IFAC 2020 Final Submission Plots:

% PosCmd = ModelResp;

close all
%Trajectory plot, Designed SAC, LQR
figure(11)
% plot(ModelResp.Data(:,1),ModelResp.Data(:,2),'--k')
plot(PosCmd.Data(:,1),PosCmd.Data(:,2),'--k')
grid on
hold on
plot(LQR.Traj.Data(:,1),LQR.Traj.Data(:,2),'r')
plot(-Hand.Traj.Data(:,1),-Hand.Traj.Data(:,2),'k')
hold off
axis equal
xlabel('x position (m)')
ylabel('y position (m)')
legend('Desired Trajectory','LQR','SAC','location','SW')
pdfplot('SwarmOptFig1')

%Error Plot for both:
%Interpolating function:
PosComAt = @(t,i) interp1(PosCmd.Time,PosCmd.Data(:,i),t);
figure(12)
plot(LQR.Error.Time - LQR.Error.Time(1),sqrt(sum(LQR.Error.Data.^2,2)),'r')
grid on
hold on
plot(Hand.Error.Time - Hand.Error.Time(1),sqrt(sum(Hand.Error.Data.^2,2)),'k')
hold off
xlabel('Time (s)')
ylabel('Position Error (m)')
legend('LQR','SAC')
ylim([0,1])
pdfplot('SwarmOptFig2')

%Control Outputs:
FirstThrust = find(xor(Hand.ThrustOut.Time >= Hand.Error.Time(1),[0;Hand.ThrustOut.Time(1:end-1)] >= Hand.Error.Time(1)))
figure(13)
plot(LQR.ThrustOut.Time(FirstThrust:end) - LQR.ThrustOut.Time(FirstThrust),sqrt(sum(LQR.ThrustOut.Data(FirstThrust:end,:).^2,2)),'r')
grid on
hold on
plot(Hand.ThrustOut.Time(FirstThrust:end) - Hand.ThrustOut.Time(FirstThrust),sqrt(sum(Hand.ThrustOut.Data(FirstThrust:end,:).^2,2)),'k')
hold off
xlabel('Time (s)')
ylabel('Thrust Command Magnitude (N)')
legend('LQR','SAC')
ylim([0,0.5])
xlim([0,200])
pdfplot('SwarmOptFig7')

%Repeat for DE and SADE:
%Trajectory plot, Designed DE and SADE
figure(21)
plot(ModelResp.Data(:,1),ModelResp.Data(:,2),'--k')
grid on
hold on
plot(-DE.Traj.Data(:,1),-DE.Traj.Data(:,2),'k')
plot(-SADE.Traj.Data(:,1),-SADE.Traj.Data(:,2),'r')
hold off
axis equal
xlabel('x position (m)')
ylabel('y position (m)')
legend('Desired Trajectory','DE','SaDE','location','SW')
pdfplot('SwarmOptFig3')

%Error Plot for both:
%Interpolating function:
figure(22)
plot(DE.Error.Time - DE.Error.Time(1),sqrt(sum(DE.Error.Data.^2,2)),'k')
grid on
hold on
plot(SADE.Error.Time - SADE.Error.Time(1),sqrt(sum(SADE.Error.Data.^2,2)),'r')
hold off
xlabel('Time (s)')
ylabel('Position Error (m)')
legend('DE','SaDE')
ylim([0,1])
pdfplot('SwarmOptFig4')

figure(23)
plot(DE.ThrustOut.Time(FirstThrust:end) - DE.Error.Time(FirstThrust),sqrt(sum(DE.ThrustOut.Data(FirstThrust:end,:).^2,2)),'k')
grid on
hold on
plot(SADE.ThrustOut.Time(FirstThrust:end) - SADE.Error.Time(FirstThrust),sqrt(sum(SADE.ThrustOut.Data(FirstThrust:end,:).^2,2)),'r')
hold off
xlabel('Time (s)')
ylabel('Thrust Command Magnitude (N)')
legend('DE','SaDE')
ylim([0,0.5])
xlim([0,200])
pdfplot('SwarmOptFig8')

%Repeat for PSO and SPSO:
%Trajectory plot, Designed PSO and SPSO
figure(31)
plot(ModelResp.Data(:,1),ModelResp.Data(:,2),'--k')
grid on
hold on
plot(-PSO.Traj.Data(:,1),-PSO.Traj.Data(:,2),'k')
plot(-SPSO.Traj.Data(:,1),-SPSO.Traj.Data(:,2),'r')
hold off
axis equal
xlabel('x position (m)')
ylabel('y position (m)')
legend('Desired Trajectory','PSO','SPSO','location','SW')
pdfplot('SwarmOptFig5')

%Error Plot for both:
%Interpolating function:
figure(32)
plot(PSO.Error.Time - PSO.Error.Time(1),sqrt(sum(PSO.Error.Data.^2,2)),'k')
grid on
hold on
plot(SPSO.Error.Time - SPSO.Error.Time(1),sqrt(sum(SPSO.Error.Data.^2,2)),'r')
hold off
xlabel('Time (s)')
ylabel('Position Error (m)')
legend('PSO','SPSO')
ylim([0,1])
pdfplot('SwarmOptFig6')

figure(33)
plot(PSO.ThrustOut.Time(FirstThrust:end) - PSO.Error.Time(FirstThrust),sqrt(sum(PSO.ThrustOut.Data(FirstThrust:end,:).^2,2)),'k')
grid on
hold on
plot(SPSO.ThrustOut.Time(FirstThrust:end) - SPSO.Error.Time(FirstThrust),sqrt(sum(SPSO.ThrustOut.Data(FirstThrust:end,:).^2,2)),'r')
hold off
xlabel('Time (s)')
ylabel('Thrust Command Magnitude (N)')
legend('PSO','SPSO')
ylim([0,0.5])
xlim([0,200])
pdfplot('SwarmOptFig9')

figure(41)
plot(ModelResp.Data(:,1),ModelResp.Data(:,2),'--k')
grid on
hold on
plot(-Hand.Traj.Data(:,1),-Hand.Traj.Data(:,2),'k')
plot(-DE.Traj.Data(:,1),-DE.Traj.Data(:,2),'r')
hold off
axis equal
xlabel('x position (m)')
ylabel('y position (m)')
legend('Desired Trajectory','Designed SAC','DE Optimized SAC','location','SW')
pdfplot('SwarmOptFig10')





