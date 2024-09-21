clear;
clc;
close all;

  % owl.assignMarker(tracker_id_RED, 5, "5", tracker_id_RED_5_pos_string); // top left
  % owl.assignMarker(tracker_id_RED, 3, "3", tracker_id_RED_3_pos_string); // top right
  % owl.assignMarker(tracker_id_RED, 1, "1", tracker_id_RED_1_pos_string); // bottom right
  % owl.assignMarker(tracker_id_RED, 7, "7", tracker_id_RED_7_pos_string); // bottom left 

datafile = importdata("attempt1.csv");

Time = datafile.data(:,1);
LAG = datafile.data(:,2);
MarkerID = datafile.data(:,3);
MarkerX = datafile.data(:,4);
MarkerY = datafile.data(:,5);

UniqueMarkers = unique(MarkerID);

Marker1X = MarkerX(MarkerID == UniqueMarkers(1));
Marker1Y = MarkerY(MarkerID == UniqueMarkers(1));

Marker2X = MarkerX(MarkerID == UniqueMarkers(2));
Marker2Y = MarkerY(MarkerID == UniqueMarkers(2));

Marker3X = MarkerX(MarkerID == UniqueMarkers(3));
Marker3Y = MarkerY(MarkerID == UniqueMarkers(3));

Marker4X = MarkerX(MarkerID == UniqueMarkers(4));
Marker4Y = MarkerY(MarkerID == UniqueMarkers(4));

index = 14958;

% figure()
% hold on
% scatter([Marker1X(index)],...
%     [Marker1Y(index)])
% 
% scatter([Marker2X(index)],...
%     [Marker2Y(index)])
% 
% scatter([ Marker3X(index)],...
%     [Marker3Y(index)])
% 
% scatter([Marker4X(index)],...
%     [Marker4Y(index)])
% legend('1 (3)', '3 (2)', '5 (1)', '7 (4)')

% For attempt2, time vec is 1 second over 181625-177805 samples
t_step = (1/(181625-177805))*4;
timeVec = linspace(0,length(Marker1X)*t_step, length(Marker1X))';

% figure()
% plot(timeVec, Marker1X)

packet = [timeVec, Marker3X, Marker3Y, zeros(size(Marker3X)),...
                   Marker4X, Marker4Y, zeros(size(Marker4X)),...
                   Marker1X, Marker1Y, zeros(size(Marker1X)),...
                   Marker2X, Marker2Y, zeros(size(Marker2X))];

writematrix(packet, '2024-05-17 Calibration.csv');

figure()
for index = 1:100:length(Marker3X)
    cla;
    hold on
    scatter([Marker1X(index)],...
        [Marker1Y(index)],'filled')
    
    scatter([Marker2X(index)],...
        [Marker2Y(index)],'filled')
    
    scatter([ Marker3X(index)],...
        [Marker3Y(index)],'filled')
    
    scatter([Marker4X(index)],...
        [Marker4Y(index)],'filled')

    axis([0 3000 0 3000]);

    %legend('1 (3)', '3 (2)', '5 (1)', '7 (4)')

    %pause(0.1);
    drawnow;
    fprintf([num2str(index) '\n']);
end