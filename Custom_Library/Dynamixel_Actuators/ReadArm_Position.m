classdef ReadArm_Position < realtime.internal.SourceSampleTime ...
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

    end
    
    properties (Nontunable)
        % Public, non-tunable properties.
    end
    
    properties (Access = private)
        % Pre-computed constants.
    end
    
    methods
        % Constructor
        function obj = ReadArm_Position(varargin)
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
            end
        end
        
        function [y1, y2, y3] = stepImpl(obj)
            
            y1  = double(0);
            y2  = double(0);
            y3  = double(0);

            if isempty(coder.target)
                % Place simulation output code here
            else
                coder.cinclude('dynamixel_sdk.h');
                coder.cinclude('dynamixel_functions.h');
                coder.ceval('read_dynamixel_position',coder.ref(y1),...
                             coder.ref(y2),coder.ref(y3));

            end
        end
        
        function releaseImpl(obj) %#ok<MANU>
            if isempty(coder.target)
                % Place simulation termination code here
            else
                % Call C-function implementing device termination
            end
        end
    end
    
    methods (Access=protected)
        %% Define output properties
        function num = getNumInputsImpl(~)
            num = 0;
        end
        
        function num = getNumOutputsImpl(~)
            num = 3;
        end
        
        function flag = isOutputSizeLockedImpl(~,~)
            flag = true;
        end
        
        function varargout = isOutputFixedSizeImpl(~,~)
            varargout{1} = true;
            varargout{2} = true;
            varargout{3} = true;
        end
        
        function flag = isOutputComplexityLockedImpl(~,~)
            flag = true;
        end
        
        function varargout = isOutputComplexImpl(~)
            varargout{1} = false;
            varargout{2} = false;
            varargout{3} = false;
        end
        
        function varargout = getOutputSizeImpl(~)
            varargout{1} = [1,1];
            varargout{2} = [1,1];
            varargout{3} = [1,1];
        end
        
        function varargout = getOutputDataTypeImpl(~)
            varargout{1} = 'double';
            varargout{2} = 'double';
            varargout{3} = 'double';
        end
        
        function icon = getIconImpl(~)
            % Define a string as the icon for the System block in Simulink.
            icon = 'ReadArm_Position';
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
            header = matlab.system.display.Header('ReadArm_Position','Title',...
                'Read Dynamixel - Joint Angles','Text',...
                ['This block read the joint angles from the encoders on the'...
                 ' dynamixel actuators. The output is the 3 joint angles in radians.' newline]);
        end
        
    end
    
    methods (Static)
        function name = getDescriptiveName()
            name = 'ReadArm_Position';
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
