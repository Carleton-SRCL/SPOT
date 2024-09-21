classdef GPIO_Write < matlab.System & coder.ExternalDependency
    %
    % System object template for a GPIO_Write block.
    % 
    % This template includes most, but not all, possible properties,
    % attributes, and methods that you can implement for a System object in
    % Simulink.
    %
    
    % Copyright 2021 The MathWorks, Inc.
    %#codegen
    %#ok<*EMCA>
    
    properties
        % Specify custom variable names
        gpioPin = 428; 
        pinDirection = 1;

    end 
    
    properties (Nontunable)
        % Public, non-tunable properties.
    end
    
    methods
        % Constructor
        function obj = GPIO_Write(varargin)
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
                coder.cinclude('gpio_control.h');
                coder.ceval('export_gpio', obj.gpioPin);
                coder.ceval('set_pin_direction', obj.gpioPin, obj.pinDirection);
            end
        end
        
        function stepImpl(obj, u)  
            if isempty(coder.target)
                % Place simulation output code here 
            else
                % Call C-function implementing device output
                coder.cinclude('gpio_control.h');
                %coder.ceval('export_gpio', obj.gpioPin);
                coder.ceval('set_pin_direction', obj.gpioPin, obj.pinDirection);
                coder.ceval('change_gpio_value', obj.gpioPin, u);
            end
        end
        
        function releaseImpl(obj)
            if isempty(coder.target)
                % Place simulation termination code here
            else
                % Call C-function implementing device termination
                coder.ceval('change_gpio_value', obj.gpioPin, 0);
                coder.ceval('unexport_gpio', obj.gpioPin);
            end
        end
    end
    
    methods (Access=protected)
        %% Define input properties
        function num = getNumInputsImpl(~)
            num = 1;
        end
        
        function num = getNumOutputsImpl(~)
            num = 0;
        end
        
        function flag = isInputSizeMutableImpl(~,~)
            flag = false;
        end
        
        function flag = isInputComplexityMutableImpl(~,~)
            flag = false;
        end
        
        function validateInputsImpl(~, u)
            if isempty(coder.target)
                % Run input validation only in Simulation
                validateattributes(u,{'double'},{'scalar'},'','u');
            end
        end
        
        function icon = getIconImpl(~)
            % Define a string as the icon for the System block in Simulink.
            icon = 'GPIO_Write';
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
            header = matlab.system.display.Header('GPIO_Write','Title',...
                'Debugging Block','Text',...
                ['This block allows you to control the GPIO pins on the NVIDIA Jetson Xavier NX Development board.' ...
                ' It is important to be aware that the pin numbers needed for this block do not correspond to the pin numbers used in most'...
                ' online resources. Here is the list of pin numbers and what they control:' newline newline ...
                '436: Thruster #1' newline...
                '445: Thruster #2' newline...
                '480: Thruster #3' newline...
                '268: Thruster #4' newline...
                '484: Thruster #5' newline...
                '483: Thruster #6' newline...
                '481: Thruster #7' newline...
                '491: Thruster #8' newline...
                '428: Pucks' newline newline ... 
                'For a complete list of all GPIO pin numbers available and their alternate functions, refer to this online resource:' newline newline ...
                'https://jetsonhacks.com/nvidia-jetson-xavier-nx-gpio-header-pinout/']);
        end
        
    end
    
    methods (Static)
        function name = getDescriptiveName()
            name = 'GPIO_Write';
        end

        function b = isSupportedContext(context)
            b = context.isCodeGenTarget('rtw');
        end
        
        function updateBuildInfo(buildInfo, context)
            if context.isCodeGenTarget('rtw')
                % Update buildInfo
                srcDir = fullfile(fileparts(mfilename('fullpath')),'src');
                includeDir = fullfile(fileparts(mfilename('fullpath')),'include');
                addIncludePaths(buildInfo, includeDir);
                addSourceFiles(buildInfo, 'gpio_control.cpp', srcDir);
                addIncludeFiles(buildInfo, 'gpio_control.h', includeDir);
            end
        end
    end
end
