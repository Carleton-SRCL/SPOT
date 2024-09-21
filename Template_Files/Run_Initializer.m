% The following script is the initializer for SPOT 4.1; in this script,
% users define all initials parameters and/or constants required for
% simulation and experiment.

clear;
clc;
close all force;

warning('off','all')

%% Start the graphical user interface or set the appropriate variables:

% No matter what, the GUI needs to be loaded
appHandle = GUI_v4_1_Main;

%% Place any custom variables or overwriting variables in this section

% As an example, here are the control parameters the manipulator.
% Set torque limits on joints

Tz_lim_sharm                   = .1; % Shoulder Joint [Nm]

Tz_lim_elarm                   = .1; % Elbow Joint [Nm]

Tz_lim_wrarm                   = .1; % Wrist Joint [Nm]

% Transpose Jacobian controller gains:

Kp = [0.08 0 0
      0    0.08 0
      0    0    0.002];
Kv = [0.05 0 0
      0    0.05 0
      0    0    0.005];

% Initialize the PID gains for the ARM:

Kp_sharm                       = 1.5;
Kd_sharm                       = 1.0;

Kp_elarm                       = 1.2;
Kd_elarm                       = 0.8;

Kp_wrarm                       = 1.0;
Kd_wrarm                       = 0.6;

% Define the model properties for the joint friction:
% Based on https://ieeexplore.ieee.org/document/1511048

%Shoulder
Gamma1_sh = 0.005; 
Gamma2_sh = 5;
Gamma3_sh = 40;
Gamma4_sh = 0.015; 
Gamma5_sh = 800; 
Gamma6_sh = 0.005;

%Elbow
Gamma1_el = 0.12; 
Gamma2_el = 5;
Gamma3_el = 10;
Gamma4_el = 0.039; 
Gamma5_el = 800;
Gamma6_el = 0.000001;

%Wrist
Gamma1_wr = 0.025;
Gamma2_wr = 5;
Gamma3_wr = 40;
Gamma4_wr = 0.029;
Gamma5_wr = 800; 
Gamma6_wr = 0.02;

%% This section of the code contains parameters should not be modified

% Set the PWM frequency
PWMFreq = 5; %[Hz]

% Set estimated thruster forces
REDFXNominal   = 0.2825;
REDFYNominal   = 0.2825;
BLACKFXNominal = 0.2825;
BLACKFYNominal = 0.2825;
BLUEFXNominal  = 0.2825;
BLUEFYNominal  = 0.2825;

%% For those who want to run simulations without using the GUI:

% % Set the diagram to run
% appHandle.AvailableDiagramsDropDown.Value = "Template_v4_1_0_2024b_Jetson.slx";
% 
% % Ensure the diagram is loaded
% open(appHandle.AvailableDiagramsDropDown.Value);
% 
% % Edit active platforms
% appHandle.REDCheckBox.Value    = 1;
% appHandle.BLACKCheckBox.Value  = 0;
% appHandle.BLUECheckBox.Value   = 0;
% appHandle.ARMCheckBox.Value    = 0;
% 
% appHandle.ConfirmSettings();
% 
% % Edit initial conditions
% appHandle.SubAppInitialConditions.REDInitialX.Value  = 1.2;  % [m]
% appHandle.SubAppInitialConditions.REDInitialY.Value  = 1.2;  % [m]
% appHandle.SubAppInitialConditions.REDInitialTh.Value = 90;   % [deg]
% 
% appHandle.SubAppInitialConditions.UpdateInitialConditions();
% 
% % Edit mass properties
% appHandle.SubAppMassProperties.OverridePropertiesCheckBox.Value = 1;
% appHandle.SubAppMassProperties.MassRedEditField.Value = 12.035;    % [kg]
% appHandle.SubAppMassProperties.InertiaRedEditField.Value = 0.19854;% [kgm2]
% 
% appHandle.SubAppMassProperties.UpdateMassProperties();
% 
% % Edit phase durations
% appHandle.SubPhase1EditField.Value = 10;        % [s]
% appHandle.SubPhase2EditField.Value = 5;        % [s]
% appHandle.SubPhase3EditField.Value = 28;        % [s]
% appHandle.SubPhase4EditField.Value = 115;        % [s]
% appHandle.DurPhase0EditField.Value = 10;                  % [s]
% appHandle.DurPhase1EditField.Value = 5;                  % [s]
% appHandle.DurPhase2EditField.Value = 40;                  % [s]
% appHandle.DurPhase4EditField.Value = 30;                  % [s]
% appHandle.DurPhase5EditField.Value = 20;                  % [s] 
% 
% appHandle.UpdateTimes();
% 
% % Execute a simulation
% appHandle.RunSimulationPublicFcn();
% 
% % Manipulate the simulation data
% figure()
% plot(dataClass.Time_s.Data, dataClass.RED_Px_m.Data,'-k')
% grid on
% hold on
% axis tight
% xlabel('Time [s]')
% ylabel('Position - X [m]')


