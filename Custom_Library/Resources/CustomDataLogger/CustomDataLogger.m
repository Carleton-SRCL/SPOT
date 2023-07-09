classdef CustomDataLogger < matlab.System ...
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
        function obj = CustomDataLogger(varargin)
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
                 coder.cinclude('custom_data_logger.h')
                 coder.ceval('createFile');
            end
        end
        
        function stepImpl(obj,u1,u2)  
            if isempty(coder.target)
                % Place simulation output code here 
            else
                % Call C-function implementing device output
                 coder.cinclude('custom_data_logger.h')
                 coder.ceval('appendDataToFile',u1, u2);
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
            num = 2;
        end
        
        function num = getNumOutputsImpl(~)
            num = 0;
        end
        
        function varargout = isInputFixedSizeImpl(~,~)
            varargout{1} = true;
        end
        
        function flag = isInputComplexityLockedImpl(~,~)
            flag = true;
        end
              
        function icon = getIconImpl(~)
            % Define a string as the icon for the System block in Simulink.
            icon = 'CustomDataLogger';
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
            header = matlab.system.display.Header('CustomDataLogger','Title',...
                'CustomDataLogger Block','Text',...
                ['This simulink block takes an input array and the size of the array and appends the data to a text file.' newline]);
        end
        
    end
    
    methods (Static)
        function name = getDescriptiveName()
            name = 'CustomDataLogger';
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
                addSourceFiles(buildInfo,'custom_data_logger.cpp', srcDir);
                
                % Add all INCLUDE files for compiling robotis software
                addIncludeFiles(buildInfo,'custom_data_logger.h',includeDir);

                %addLinkFlags(buildInfo,{'-lSource'});
                %addLinkObjects(buildInfo,'sourcelib.a',srcDir);
                %addCompileFlags(buildInfo,{'-D_DEBUG=1'});
                %addDefines(buildInfo,'MY_DEFINE_1')
            end
        end
    end
end
