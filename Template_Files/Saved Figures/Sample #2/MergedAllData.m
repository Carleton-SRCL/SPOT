clear;
clc;
close('all');

% Load in data files:

RED   = load([cd '\ExperimentData_RED_2022_6_14_13_40\Template_v3_08_2020a_1_1.mat']);
BLUE  = load([cd '\ExperimentData_BLUE_2022_6_14_13_40\Template_v3_08_2020a_1_1.mat']);
BLACK = load([cd '\ExperimentData_BLACK_2022_6_14_13_40\Template_v3_08_2020a_1_1.mat']);

% Align time vectors using timeseries:

TimeSeriesRED   = timeseries(RED.rt_dataPacket(:,2:end),RED.rt_dataPacket(:,1));
TimeSeriesBLACK = timeseries(BLACK.rt_dataPacket(:,2:end),BLACK.rt_dataPacket(:,1));
TimeSeriesBLUE  = timeseries(BLUE.rt_dataPacket(:,2:end),BLUE.rt_dataPacket(:,1));

[TimeSeriesRED,TimeSeriesBLACK] = synchronize(TimeSeriesRED,TimeSeriesBLACK,'intersection'); 
[TimeSeriesRED,TimeSeriesBLUE] = synchronize(TimeSeriesRED,TimeSeriesBLUE,'intersection'); 
[TimeSeriesRED,TimeSeriesBLACK] = synchronize(TimeSeriesRED,TimeSeriesBLACK,'intersection'); 

% Loop through all arrays. If a column average is exactly 0, merged:

for i = 1:size(TimeSeriesRED.data,2)
   
    ColumnMean = mean(TimeSeriesRED.data(:,i));
    
    if ColumnMean == 0
        
        fprintf(['Column ' num2str(i) ' is empty.\n']);
        
        ColumnMeanBLACK = mean(TimeSeriesBLACK.data(:,i));
        ColumnMeanBLUE  = mean(TimeSeriesBLUE.data(:,i));
        
        if ColumnMeanBLACK ~= 0 && ColumnMeanBLUE == 0
            
            TimeSeriesRED.data(:,i) = TimeSeriesBLACK.data(:,i);
            
        elseif ColumnMeanBLACK == 0 && ColumnMeanBLUE ~= 0
                
            TimeSeriesRED.data(:,i) = TimeSeriesBLUE.data(:,i);  
            
        else
            
            % do nothing
            
        end
                     
        
    end
       
end

dataClass = ApplyDataDictionary([TimeSeriesRED.time, TimeSeriesRED.data]);

save([cd '\ExperimentData_Merged_2022_6_14_13_40.mat'],'dataClass');