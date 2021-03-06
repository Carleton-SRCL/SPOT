classdef STREAMDATA_RB < matlab.System ...
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
        function obj = STREAMDATA_RB(varargin)
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
                 coder.cinclude('UDP_Client_Headers.h');
                 coder.ceval('Initialize_Client');
            end
        end
        
        function y = stepImpl(obj)
            y1  = double(0); % TIME
            y2  = double(0); % RED PX
            y3  = double(0); % RED PY
            y4  = double(0); % RED ATT
            y5  = double(0); % BLACK PX
            y6  = double(0); % BLACK PY
            y7  = double(0); % BLACK ATT
            y8  = double(0); % ARM ELB X
            y9  = double(0); % ARM ELB Y
            y10 = double(0); % ARM WRS X
            y11 = double(0); % ARM WRS X
            y12 = double(0); % ARM WRS X
            y13 = double(0); % ARM WRS X
            
            y  = zeros(1,13);
            
            if isempty(coder.target)
                % Place simulation output code here
            else      
                coder.ceval('Receive_UDP_Packet',coder.ref(y1), coder.ref(y2),...
                    coder.ref(y3), coder.ref(y4), coder.ref(y5), coder.ref(y6),...
                    coder.ref(y7), coder.ref(y8), coder.ref(y9), coder.ref(y10),...
                    coder.ref(y11), coder.ref(y12), coder.ref(y13));
                y = [y1, y2/1000, y3/1000, y4, y5/1000, y6/1000, y7, y8/1000, y9/1000, y10/1000,...
                     y11/1000, y12/1000, y13/1000];
            end
        end
        
        function releaseImpl(obj) %#ok<MANU>
            if isempty(coder.target)
                % Place simulation termination code here
            else
                % Call C-function implementing device termination
                coder.ceval('Terminate_SocketConnection');
            end
        end
    end
    
    methods (Access=protected)
        %% Define output properties
        function num = getNumInputsImpl(~)
            num = 0;
        end
        
        function num = getNumOutputsImpl(~)
            num = 1;
        end
        
        function flag = isOutputSizeLockedImpl(~,~)
            flag = true;
        end
        
        function varargout = isOutputFixedSizeImpl(~,~)
            varargout{1} = true;
        end
        
        function flag = isOutputComplexityLockedImpl(~,~)
            flag = true;
        end
        
        function varargout = isOutputComplexImpl(~)
            varargout{1} = false;
        end
        
        function varargout = getOutputSizeImpl(~)
            varargout{1} = [1,13];
        end
        
        function varargout = getOutputDataTypeImpl(~)
            varargout{1} = 'double';
        end
        
        function icon = getIconImpl(~)
            % Define a string as the icon for the System block in Simulink.
            icon = 'STREAMDATA_RB';
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
            header = matlab.system.display.Header('STREAMDATA_RB','Title',...
                'Receive PhaseSpace Data via UDP','Text',...
                ['This simulink block receives the PhasesSpace data from the UDP server. '...
                'This block should be used once.' newline]);
        end
        
    end
    
    methods (Static)
        function name = getDescriptiveName()
            name = 'STREAMDATA_RB';
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
                addIncludeFiles(buildInfo,'UDP_Client_Headers.h',includeDir);
                
                % Add the SOURCE files for the PhaseSpace camera 
                addSourceFiles(buildInfo,'UDP_Client_Functions.cpp',srcDir);
                
                %addLinkFlags(buildInfo,{'-lSource'});
                %addLinkObjects(buildInfo,'sourcelib.a',srcDir);
                %addCompileFlags(buildInfo,{'-D_DEBUG=1'});
                %addDefines(buildInfo,'MY_DEFINE_1')
            end
        end
    end
end
