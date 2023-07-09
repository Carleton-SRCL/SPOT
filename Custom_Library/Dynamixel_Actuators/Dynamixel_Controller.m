classdef Dynamixel_Controller < realtime.internal.SourceSampleTime ...
        & coder.ExternalDependency ...
        & matlab.system.mixin.Propagates ...
        & matlab.system.mixin.CustomIcon
    %
    % System object template for a source block.
    % 
    % This template includes most, but not all, possible properties,
    % attributes, and methods that you can implement for a System object in
    % Simulink.
    %
    % NOTE: When renaming the class name Source, the file name and
    % constructor name must be updated to use the class name.
    %
    
    % Copyright 2016 The MathWorks, Inc.
    %#codegen
    %#ok<*EMCA>
    
    properties
        
        POSITION_P_GAIN   = 400;
        POSITION_I_GAIN   = 0;
        POSITION_D_GAIN   = 200;
        MAX_POSITION      = 3072;
        MIN_POSITION      = 1024;
        MOVE_TIME         = 0;
        CURRENT_LIMIT     = 850;
        SPEED_P_GAIN      = 100;
        SPEED_I_GAIN      = 1920;
        VELOCITY_LIMIT    = 1023;
        ACCELERATION_TIME = 0;

    end
    
    properties (Nontunable)
        % Public, non-tunable properties.

    end
    
    properties (Access = private)
        % Pre-computed constants.
        run_once_flag = true;
    end
    
    methods
        % Constructor
        function obj = Dynamixel_Controller(varargin)
            % Support name-value pair arguments when constructing the object.
            setProperties(obj,nargin,varargin{:});
        end
    end
    
    
    methods (Access=protected)
        function setupImpl(obj) 
            if isempty(coder.target)
                % Place simulation setup code here
            else
                 coder.cinclude('dynamixel_sdk.h');
                 coder.cinclude('dynamixel_functions.h');
                 coder.ceval('initialize_dynamixel');
            end
        end
        
        function stepImpl(obj,Control_Mode, Joint1_Command, Joint2_Command, Joint3_Command)  
            if isempty(coder.target)
                % Place simulation output code here 
            else
                % Call C-function implementing device output

                % include the dynamicel functions
                coder.cinclude('dynamixel_sdk.h');
                coder.cinclude('dynamixel_functions.h');

                % Run the main controller code. If the switch state is
                % true then this code will initialize the parameters and
                % then start the actuator, and THEN run the command. If the
                % switch state is false, it will not reinitialize the
                % motor
                coder.ceval('dynamixel_controller',Control_Mode, obj.POSITION_P_GAIN, obj.POSITION_I_GAIN, obj.POSITION_D_GAIN, obj.MAX_POSITION,...
                                   obj.MIN_POSITION, obj.MOVE_TIME, Joint1_Command, ...
                                   Joint2_Command, Joint3_Command, obj.CURRENT_LIMIT, Joint1_Command, Joint2_Command, Joint3_Command, obj.SPEED_P_GAIN,...
                                   obj.SPEED_I_GAIN, obj.VELOCITY_LIMIT, Joint1_Command, Joint2_Command, Joint3_Command, ...
                                   obj.ACCELERATION_TIME);
                       
                               
            end
        end
        
        function releaseImpl(obj) %#ok<MANU>
            if isempty(coder.target)
                % Place simulation termination code here
            else
                 coder.cinclude('dynamixel_sdk.h');
                 coder.cinclude('dynamixel_functions.h');
                 coder.ceval('terminate_dynamixel');
            end
        end
    end
    
    methods (Access=protected)

        %% Define input properties
        function num = getNumInputsImpl(~)
            num = 4;
        end
                
        function flag = isInputSizeLockedImpl(~,~)
            flag = true;
        end
        
        function varargout = isInputFixedSizeImpl(~,~)
            varargout{1} = true;
        end
        
        function flag = isInputComplexityLockedImpl(~,~)
            flag = true;
        end
                       
        function icon = getIconImpl(~)
            % Define a string as the icon for the System block in Simulink.
            icon = 'Dynamixel_Controller';
        end    
    end
    
    methods (Static, Access=protected)
        function simMode = getSimulateUsingImpl(~)
            simMode = 'Interpreted execution';
        end
        
        function isVisible = showSimulateUsingImpl
            isVisible = false;
        end
        
       function header = getHeaderImpl
            header = matlab.system.display.Header('Dynamixel_Controller','Title',...
                'Control Dynamixel Actuators','Text',...
                ['This block takes in three inputs and sets up the actuators. If the control mode is 1, then'...
                 ' the actuators are in position control mode; 2 is current control mode; 3 is speed control mode.' ...
                 ' If the control mode is set to 0, this is a special control mode which is required to switch control modes ' ...
                 ' dynamically. In other words, before changing from torque to position within the same experiment, ' ...
                 ' the user needs to first send a command mode 0 signal to reset the actuator.' newline newline...
                 'In position control mode, the inputs are expected to be doubles and in units of radians; for'...
                 ' current control, they are integers in units of bits; for speed control they are doubles in radians per second.' newline newline]);
        end
        
    end
    
    methods (Static)
        function name = getDescriptiveName()
            name = 'Dynamixel_Controller';
        end
        
        function b = isSupportedContext(context)
            b = context.isCodeGenTarget('rtw');
        end
        
        function updateBuildInfo(buildInfo, context)
            if context.isCodeGenTarget('rtw')
                
                % Update buildInfo
                srcDir = fullfile(fileparts(mfilename('fullpath')),'src');
                includeDir = fullfile(fileparts(mfilename('fullpath')),'include');
                addIncludePaths(buildInfo,includeDir);
                
                % Add all SOURCE files for compiling robotis software
                addSourceFiles(buildInfo,'protocol2_packet_handler.cpp', srcDir);
                addSourceFiles(buildInfo,'protocol1_packet_handler.cpp', srcDir);
                addSourceFiles(buildInfo,'port_handler_windows.cpp', srcDir);
                addSourceFiles(buildInfo,'port_handler_mac.cpp', srcDir);
                addSourceFiles(buildInfo,'port_handler_linux.cpp', srcDir);
                addSourceFiles(buildInfo,'port_handler_arduino.cpp', srcDir);
                addSourceFiles(buildInfo,'port_handler.cpp', srcDir);
                addSourceFiles(buildInfo,'packet_handler.cpp', srcDir);
                addSourceFiles(buildInfo,'group_sync_write.cpp', srcDir);
                addSourceFiles(buildInfo,'group_sync_read.cpp', srcDir);
                addSourceFiles(buildInfo,'group_bulk_write.cpp', srcDir);
                addSourceFiles(buildInfo,'group_bulk_read.cpp', srcDir);
                addSourceFiles(buildInfo,'dynamixel_functions.cpp', srcDir);
                
                % Add all INCLUDE files for compiling robotis software
                addIncludeFiles(buildInfo,'protocol2_packet_handler.h',includeDir);
                addIncludeFiles(buildInfo,'protocol1_packet_handler.h',includeDir);
                addIncludeFiles(buildInfo,'port_handler_windows.h',includeDir);
                addIncludeFiles(buildInfo,'port_handler_mac.h',includeDir);
                addIncludeFiles(buildInfo,'port_handler_linux.h',includeDir);
                addIncludeFiles(buildInfo,'port_handler.h',includeDir);
                addIncludeFiles(buildInfo,'packet_handler.h',includeDir);
                addIncludeFiles(buildInfo,'group_sync_write.h',includeDir);
                addIncludeFiles(buildInfo,'group_sync_read.h',includeDir);
                addIncludeFiles(buildInfo,'group_bulk_write.h',includeDir);
                addIncludeFiles(buildInfo,'group_bulk_read.h',includeDir);
                addIncludeFiles(buildInfo,'dynamixel_sdk.h',includeDir);
                addIncludeFiles(buildInfo,'port_handler_arduino.h',includeDir);
                addIncludeFiles(buildInfo,'dynamixel_functions.h',includeDir)
                
                %addLinkFlags(buildInfo,{'-lSource'});
                %addLinkObjects(buildInfo,'sourcelib.a',srcDir);
                %addCompileFlags(buildInfo,{'-D_DEBUG=1'});
                %addDefines(buildInfo,'MY_DEFINE_1')
            end
        end
    end
end
