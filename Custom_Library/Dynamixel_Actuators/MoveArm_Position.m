classdef MoveArm_Position < matlab.System ...
        & coder.ExternalDependency ...
        & matlab.system.mixin.Propagates ...
        & matlab.system.mixin.CustomIcon
    %
    % System object template for a sink block.
    % 
    % This template includes most, but not all, possible properties,
    % attributes, and methods that you can implement for a System object in
    % Simulink.
    %
    % NOTE: When renaming the class name Sink, the file name and
    % constructor name must be updated to use the class name.
    %
    
    % Copyright 2016 The MathWorks, Inc.
    %#codegen
    %#ok<*EMCA>
    
    properties

    end
    
    properties (Nontunable)

    end
    
    properties (Access = private)

    end
    
    methods
        % Constructor
        function obj = MoveArm_Position(varargin)
            % Support name-value pair arguments when constructing the object.
            setProperties(obj,nargin,varargin{:});
        end
    end
    
    methods (Access=protected)
        function setupImpl(~)
            if isempty(coder.target)
                % Place simulation setup code here
            else
                % Call C-function implementing device initialization
            end
        end
        
        function stepImpl(~,u1,u2,u3)  
            if isempty(coder.target)
                % Place simulation output code here 
            else
                % Call C-function implementing device output
                %coder.ceval('sink_output',u);
                 coder.cinclude('dynamixel_sdk.h');
                 coder.cinclude('dynamixel_functions.h');
                 coder.ceval('command_dynamixel_position', u1, u2, u3);
            end
        end
        
        function releaseImpl(obj) %#ok<MANU>
            if isempty(coder.target)
                % Place simulation termination code here
            else
                % Call C-function implementing device termination
                %coder.ceval('sink_terminate');
            end
        end
    end
    
    methods (Access=protected)
        %% Define input properties
        function num = getNumInputsImpl(~)
            num = 3;
        end
        
        function num = getNumOutputsImpl(~)
            num = 0;
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
        
        function validateInputsImpl(~, u1, u2, u3)
            if isempty(coder.target)
                % Run input validation only in Simulation
                validateattributes(u1,{'double'},{'scalar'},'','u1');
                validateattributes(u2,{'double'},{'scalar'},'','u2');
                validateattributes(u3,{'double'},{'scalar'},'','u3');
            end
        end
        
        function icon = getIconImpl(~)
            % Define a string as the icon for the System block in Simulink.
            icon = 'MoveArm_Position';
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
            header = matlab.system.display.Header('MoveArm_Position','Title',...
                'Dynamixel Actuator - Position Control','Text',...
                ['This simulink block sends the input to the Dynamixel actuators. '...
                'The input must be the desired joint angle in radians.'...
                ' The inputs are joint #1, #2, and #3 going from top to bottom.' newline]);
        end
        
    end
    
    methods (Static)
        function name = getDescriptiveName()
            name = 'MoveArm_Position';
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
