classdef InitializeForSpecial < matlab.System ...
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
        CURRENT_LIMIT  = 850;
        P_GAIN          = 400;
        I_GAIN          = 0;
        D_GAIN          = 200;
        MAX_POSITION    = 3072;
        MIN_POSITION    = 1024;
        VELOCITY_LIMIT  = 0;
    end
    
    properties (Nontunable)
        
    end
    
    properties (Access = private)

    end
    
    methods
        % Constructor
        function obj = InitializeForSpecial(varargin)
            % Support name-value pair arguments when constructing the object.
            setProperties(obj,nargin,varargin{:});
        end
    end
    
    methods (Access=protected)
        function setupImpl(obj)
            if isempty(coder.target)
                % Place simulation setup code here
            else
                % Call C-function implementing device initialization
                 coder.cinclude('dynamixel_sdk.h');
                 coder.cinclude('dynamixel_functions.h');
                 coder.ceval('initialize_dynamixel_special_control',obj.P_GAIN, obj.I_GAIN, ...
                     obj.D_GAIN, obj.MAX_POSITION, obj.MIN_POSITION, obj.VELOCITY_LIMIT,obj.CURRENT_LIMIT);
            end
        end
        
        function stepImpl(~,~)  
            if isempty(coder.target)
                % Place simulation output code here 
            else
                % Call C-function implementing device output
            end
        end
        
        function releaseImpl(obj) %#ok<MANU>
            if isempty(coder.target)
                % Place simulation termination code here
            else
                % Call C-function implementing device termination
                % Call C-function implementing device initialization
                 coder.cinclude('dynamixel_sdk.h');
                 coder.cinclude('dynamixel_functions.h');
                 coder.ceval('terminate_dynamixel');
            end
        end
    end
    
    methods (Access=protected)
        %% Define input properties
        function num = getNumInputsImpl(~)
            num = 0;
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
        
        function validateInputsImpl(~, ~)
            if isempty(coder.target)
                % Run input validation only in Simulation
                % validateattributes(u,{'double'},{'scalar'},'','u');
            end
        end
        
        function icon = getIconImpl(~)
            % Define a string as the icon for the System block in Simulink.
            icon = 'InitializeForSpecial';
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
            header = matlab.system.display.Header('InitializeForSpecial','Title',...
                'Dynamixel Actuator - Initialize Special Control','Text',...
                ['This simulink block initializes the actuators for Special control. '...
                'This block must be placed ONCE in the diagram, at the top level.' newline]);
        end
        
    end
    
    methods (Static)
        function name = getDescriptiveName()
            name = 'InitializeForSpecial';
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
