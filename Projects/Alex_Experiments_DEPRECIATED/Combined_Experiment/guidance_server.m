classdef guidance_server < matlab.System ...
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
        % Platform Selection
        %platformID = 1;
    end
    
    properties (Nontunable)
        % Public, non-tunable properties.
    end
    
    properties (Access = private)
        % Pre-computed constants.
    end
    
    methods
        % Constructor
        function obj = guidance_server(varargin)
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
                coder.cinclude('TCP_Server.h');
                coder.ceval('createServerNewSocket');
            end
        end
        
        function [guided_acceleration_x, guided_acceleration_y, guided_angular_acceleration] = stepImpl(obj, time, red_x, red_y, red_angle, red_vx, red_vy, red_dangle,...
            black_x, black_y, black_angle,black_vx, black_vy, black_dangle)
            
            guided_acceleration_x = double(0);
            guided_acceleration_y = double(0);
            guided_angular_acceleration = double(0);
            
            if isempty(coder.target)
                % Place simulation output code here
                
            else
                %coder.ceval();
                %connected = ;
                coder.ceval("run_guidance", time, red_x, red_y, red_angle, red_vx, red_vy, red_dangle, black_x, black_y, black_angle, black_vx, black_vy, black_dangle,... 
                coder.ref(guided_acceleration_x), coder.ref(guided_acceleration_y), coder.ref(guided_angular_acceleration));
                
            end
        end
        
        function releaseImpl(obj) %#ok<MANU>
            if isempty(coder.target)
                % Place simulation termination code here
                
            else
                % Call C-function implementing device termination
                coder.ceval('terminate');
            end
        end
    end
    
    methods (Access=protected)
        %% Define output properties
        function num = getNumInputsImpl(~)
            num = 13;
        end
        
        function [input1_name, input2_name, input3_name, input4_name, input5_name, input6_name, input7_name,...
                input8_name,input9_name,input10_name,input11_name,input12_name,input13_name] = getInputNamesImpl(~)
            input1_name = "u_time";
            input2_name = "red x";
            input3_name = "red y";
            input4_name = "red angle";
            input5_name = "red vx";
            input6_name = "red vy";
            input7_name = "red dangle";
            input8_name = "black x";
            input9_name = "black y";
            input10_name = "black angle";
            input11_name = "black vx";
            input12_name = "black vy";
            input13_name = "black dangle";
        end
        
        function num = getNumOutputsImpl(~)
            num = 3;
        end
        
        function [output1_name, output2_name, output3_name] = getOutputNamesImpl(~)
            output1_name = "guided acceleration x";
            output2_name = "guided acceleration y";
            output3_name = "guided angular acceleration";
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
            varargout{1} = [1];
            varargout{2} = [1];
            varargout{3} = [1];
        end
        
        function varargout = getOutputDataTypeImpl(~)
            varargout{1} = 'double';
            varargout{2} = 'double';
            varargout{3} = 'double';
        end
        
        function icon = getIconImpl(~)
            % Define a string as the icon for the System block in Simulink.
            icon = 'guidance_server';
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
            header = matlab.system.display.Header('guidance_server','Title',...
                'TCP Server','Text',...
                ['This simulink block sends and receives data to and from the Jetson using a TCP/IP Server. '...
                'This block should be used once.' newline]);
        end
        
    end
    
    methods (Static)
        function name = getDescriptiveName()
            name = 'guidance_server';
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
                
                % Add the INCLUDE files for the PhaseSpace camera
                addIncludeFiles(buildInfo,'TCP_Server.h',includeDir);
                
                % Add the SOURCE files for the PhaseSpace camera
                addSourceFiles(buildInfo,'TCP_Server.cpp',srcDir);
                
                %addLinkFlags(buildInfo,{'-lSource'});
                %addLinkObjects(buildInfo,'sourcelib.a',srcDir);
                %addCompileFlags(buildInfo,{'-D_DEBUG=1'});
                %addDefines(buildInfo,'MY_DEFINE_1')
            end
        end
    end
end
