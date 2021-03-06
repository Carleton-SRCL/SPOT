%Interpreting Experimental Results:

RunName = 'ExperimentData_BLACK_2021_2_16_16_41';%First test run
RunDir = dir(['ExperimentalResults\',RunName]);
clearvars Run Dummy %if you don't do this it adds (#offiles-2) to the Run size, because apparently Run(1:end) is perpendicular to Run(1:end,1)...
close all

%List of output directories:
% 1 - Universal Time (UTC) sec
% 2 - RedSat X axis Thrust N
% 3 - RedSat Y axis Thrust N
% 4 - RedSat Z axis torque Nm
% 5 - RedSat X position m
% 6 - RedSat Y position m
% 7 - RedSat Z angle rad (?)
% 8 - RedSat X velocity m/s
% 9 - RedSat Y velocity m/s
% 10- RedSat Z (angular?) velocity rad/s(?)
% 11- BlackSat X axis Thrust N
% 12- BlackSat Y axis Thrust N
% 13- BlackSat Z axis torque Nm
% 14- BlackSat X position m
% 15- BlackSat Y position m
% 16- BlackSat Z angle rad (?)
% 17- BlackSat X velocity m/s
% 18- BlackSat Y velocity m/s
% 19- BlackSat Z (angular?) velocity rad/s(?)
% 20- Blacksat RzD (I don't know man)

%Load Each .mat file to its own thing:

for i = 3:size(RunDir) %First two results are '.' and '..'

%For each file, make a variable with cell array position using the last bit
%of the name:

UnderscorePositions = find(RunDir(i).name == '_');
TheTwoICareAbout = UnderscorePositions(end-1:end);
FirstPos = str2num(RunDir(i).name(TheTwoICareAbout(1)+1:TheTwoICareAbout(2)-1));%The first number
SecondPos = str2num(RunDir(i).name(TheTwoICareAbout(2)+1:end-4));%Don't forget to cut off '.mat'
%Create the structure that'll do this stuff for me:
Run(FirstPos,SecondPos) = load(['ExperimentalResults\',RunName,'\',RunDir(i).name]);

end

%Now we plot the positions:
FileCount = size(Run,1);
%omg apparently there are blank spots.
%Do I seriously have to discover how to count these MF things?

%Reorganize the File order:
ListCount = 1;
ElementList = [];
for i = 1:size(Run,1)
for j = 1:size(Run,2)

%See if a file exists in this spot.
if ~isempty(Run(i,j).rt_tout) | ~isempty(Run(i,j).rt_dataPacket)
    ElementList(ListCount,:) = [i,j];
    ListCount = ListCount + 1;
end
end
end
ListCount = ListCount-1;%Now the correct number of .mat files, useful for indexing later.

%Translates from filename to Dummy list:
Translate = @(x,y) find(ElementList(:,1) == x & ElementList(:,2) == y);

%Now Reorganize the whole Gaff:
for i = 1:size(ElementList)%Can't find a smart way of doing this
Dummy(i).rt_tout = Run(ElementList(i,1),ElementList(i,2)).rt_tout;
Dummy(i).rt_dataPacket = Run(ElementList(i,1),ElementList(i,2)).rt_dataPacket;
end
clearvars Run
%And Put it Back:
Run = Dummy;

%FINALLY I can do some analysis of the Runs:
%Create the useful Data Streams:

% 1 - time - Universal Time (UTC) sec 
% 2 - RFx  - RedSat X axis Thrust N
% 3 - RFy  - RedSat Y axis Thrust N
% 4 - RTz  - RedSat Z axis torque Nm
% 5 - RPx  - RedSat X position m
% 6 - RPy  - RedSat Y position m
% 7 - RRz  - RedSat Z angle rad (?)
% 8 - RVx  - RedSat X velocity m/s
% 9 - RVy  - RedSat Y velocity m/s
% 10- RVz  - RedSat Z (angular?) velocity rad/s(?)
% 11- BFx  - BlackSat X axis Thrust N
% 12- BFy  - BlackSat Y axis Thrust N
% 13- BTz  - BlackSat Z axis torque Nm
% 14- BPx  - BlackSat X position m
% 15- BPy  - BlackSat Y position m
% 16- BRz  - BlackSat Z angle rad (?)
% 17- BVx  - BlackSat X velocity m/s
% 18- BVy  - BlackSat Y velocity m/s
% 19- BVz  - BlackSat Z (angular?) velocity rad/s(?)
% 20- BzD  - Blacksat RzD (I don't know man)

for i = 1:ListCount %No elegant solutions here, I think
Run(i).time = (Run(i).rt_tout);
Run(i).RFx = (Run(i).rt_dataPacket(:,2));
Run(i).RFy = (Run(i).rt_dataPacket(:,3));
Run(i).RTz = (Run(i).rt_dataPacket(:,4));
Run(i).RPx = (Run(i).rt_dataPacket(:,5));
Run(i).RPy = (Run(i).rt_dataPacket(:,6));
Run(i).RRz = (Run(i).rt_dataPacket(:,7));
Run(i).RVx = (Run(i).rt_dataPacket(:,8));
Run(i).RVy = (Run(i).rt_dataPacket(:,9));
Run(i).RVz = (Run(i).rt_dataPacket(:,10));
Run(i).BFx = (Run(i).rt_dataPacket(:,11));
Run(i).BFy = (Run(i).rt_dataPacket(:,12));
Run(i).BTz = (Run(i).rt_dataPacket(:,13));
Run(i).BPx = (Run(i).rt_dataPacket(:,14));
Run(i).BPy = (Run(i).rt_dataPacket(:,15));
Run(i).BRz = (Run(i).rt_dataPacket(:,16));
Run(i).BVx = (Run(i).rt_dataPacket(:,17));
Run(i).BVy = (Run(i).rt_dataPacket(:,18));
Run(i).BzD = (Run(i).rt_dataPacket(:,19));
end

%Boom!
%Now to Compare some results:

%Create graphs for all the relevant stuff

%Booleans of graphs to turn on or off:
RedSatGraph = 0;
BlackSatGraph = 1;
RPos = 1;
RVel = 1;
RForce = 1;
BPos = 1;
BVel = 0;
BForce = 0;
BzDQuestionMark = 0;

RunList = [1,ListCount(end)];%Make this a 2 vector
% RunList = [1,1];

for i = RunList(1):RunList(2)
    
    
    %Graph RedPosition
    if RedSatGraph & RPos
       figure(i*10+1)
       plot(Run(i).RPx,Run(i).RPy)
       grid on
       
       figure(i*10+2)
       plot(Run(i).time,Run(i).RPx)
       grid on
       hold on
       plot(Run(i).time,Run(i).RPy)
       plot(Run(i).time,Run(i).RRz)
    end
    
    if RedSatGraph & RVel
        figure(i*10+3)
        plot(Run(i).time,Run(i).RVx)
        grid on
        hold on
        plot(Run(i).time,Run(i).RVy)
        plot(Run(i).time,Run(i).RVz)
    end
    
    if RedSatGraph & RForce
        figure(i*10+4)
        plot(Run(i).time,Run(i).RFx)
        grid on
        hold on
        plot(Run(i).time,Run(i).RFy)
        plot(Run(i).time,Run(i).RTz)
    end
    
    if BlackSatGraph & BPos
        figure(i*10+5)
        plot(Run(i).BPx,Run(i).BPy)
        grid on
        
        figure(i*10+6)
        plot(Run(i).time,Run(i).BPx)
        grid on
        hold on
        plot(Run(i).time,Run(i).BPy)
        plot(Run(i).time,Run(i).BRz)
    end
    
    if BlackSatGraph & BVel
        figure(i*10+7)
        plot(Run(i).time,Run(i).BVx)
        grid on
        hold on
        plot(Run(i).time,Run(i).BVy)
        plot(Run(i).time,Run(i).BVz)
    end
    
    if BlackSatGraph & BForce
        figure(i*10+8)
        plot(Run(i).time,Run(i).BFx)
        grid on
        hold on
        plot(Run(i).time,Run(i).BFy)
        plot(Run(i).time,Run(i).BTz)
    end
end


%Here I start putting run specific things to do:

if strcmp(RunName,'ExperimentData_BLACK_2021_2_16_16_41')
    
    
    %First thing is to put up the data I want to compare:
    
    %LQR response of the system
    
    %Command for the system: Just a copy from the experiment script:
    omega = 0.03;
    phi = 30;
    offset = init_states_BLACK;%Center of the table
    [track,Force,Jerk] = cycloid(t_vector,R,H,omega,0,0,phi,rotation);
    track(:,1) = track(:,1)+offset(1);
    track(:,2) = track(:,2)+offset(2);
    Path_xyz.time = t_vector;
    delaycount = 100000*0;
    Path_xyz.signals.values = [zeros(delaycount,3);track(1:end-delaycount,1:3)];
    Path_xyz.signals.dimensions = [3];
    PathAtStart = Path_xyz.signals.values(Path_xyz.time == Phase1_End,:);
    Path_xyz.signals.values = [zeros(delaycount,3);track(1:end-delaycount,1:3)];
    Path_xyz.signals.dimensions = [3];
    
    %Example model, can be tweaked:
    BLACKMass = 12.3341;
    A1 = [0,1;
        0,0];
    B1 = [0;
        1/BLACKMass];
    Plant.A = A1;
    Plant.B = B1;
    Model.C                             = [1,0];
    Model.D                             = 0;
    Plant.C = Model.C;
    Plant.D = Model.D;
    PlantTF = tf(1,[m,0,0]);%Model of TF, OL
    
    Q1 = [1,0;
        0,1];
    R1 = [10];
    [X1,L1,G1] = care (A1,B1,Q1,R1); %Awesome for 1 axis
    
    LQRTF = tf([G1(2),G1(2)]);%OL
    
    %Simple Dynamics model:
    CLLQR = feedback(series(LQRTF,PlantTF));%-ve fb assumed
    
    %Create the expected system resposne
    
    %Stitch together two data files for the successful run (10):
    RRun = stitch(Run(Translate(10,1)),Run(Translate(10,2)));
    
    %Now compare the response:
    figure(1)
    plot(RRun.BPx,RRun.BPy)
    grid on
    hold on
    plot(Path_xyz.signals.values(:,1),Path_xyz.signals.values(:,2))
    
    figure(2)
    plot(RRun.time-RRun.time(1),RRun.BPx)
    grid on
    hold on
    plot(RRun.time-RRun.time(1),RRun.BPy)
    plot(Path_xyz.time - Path_xyz.time(1),Path_xyz.signals.values(:,1))
    plot(Path_xyz.time - Path_xyz.time(1),Path_xyz.signals.values(:,2))
    %Difference signals:
    %interpolation Stuff:
    DifBPx = Path_xyz.signals.values(:,1)-interp1(RRun.time,RRun.BPx,Path_xyz.time - Path_xyz.time(1))';
    DifBPy = Path_xyz.signals.values(:,2)-interp1(RRun.time,RRun.BPy,Path_xyz.time - Path_xyz.time(1))';
    Dif2Norm = sqrt(DifBPx.*DifBPx + DifBPy.*DifBPy);
    plot(Path_xyz.time - Path_xyz.time(1),DifBPx)
    plot(Path_xyz.time - Path_xyz.time(1),DifBPy)
    plot(Path_xyz.time - Path_xyz.time(1),Dif2Norm)
    
    
    
end

































