% The following script will create a new SPOT experiment.

function [] = Setup_Experiment()

    clear;
    clc;
    close all;
    
    expName = inputdlg('Please enter the name of the experiment (no spaces):');
    
    if isempty(expName)==1
        fprintf("No name entered...\n");
    else
        copyfile([cd filesep 'Template_Files' filesep],...
        [cd filesep 'Projects' filesep expName{1,1}]);
    
        cd([cd filesep 'Projects' filesep expName{1,1}]);
    
        movefile('Template_v5_0_0_2024b_Jetson.slx',[expName{1,1} '.slx']);
    end

end
