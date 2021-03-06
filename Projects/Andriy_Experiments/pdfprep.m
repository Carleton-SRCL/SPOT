function [ ] = pdfprep()
%PDFPLOT prints off a PDF from a matlab plot...
%   Detailed explanation goes here

%Code Built for outputting GRAPHS as PDFS
% font change
set(0,'DefaultAxesFontName', 'Times New Roman') 
set(0,'DefaultAxesFontSize', 18)

pos = get(gcf, 'Position'); %// gives x left, y bottom, width, height
width = pos(3)/100;
height = pos(4)/100;

% exporting properties of figure
%width = w;     					% Width in inches
%height = h;    					% Height in inches

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
ax = gca;
outerpos = ax.OuterPosition;
ti = ax.TightInset; 
left = outerpos(1) + ti(1);
bottom = outerpos(2) + ti(2);
ax_width = outerpos(3) - ti(1) - ti(3);
ax_height = outerpos(4) - ti(2) - ti(4);
ax.Position = [left bottom ax_width ax_height];
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

fig = gcf;
fig.PaperPositionMode = 'auto';
fig_pos = fig.PaperPosition;
fig.PaperSize = [fig_pos(3) fig_pos(4)];

pos = get(gcf, 'Position');			% don't touch
set(gcf, 'Position', [pos(1) pos(2) width*100, height*100]); % don't touch
set(gcf, 'Paperposition', [0 0 width height])	% don't touch
set(gcf,'papersize',[width height])		% don't touch
%print(filename,'-dpdf','-r700');		% first entry is file name, second is type, third is resolution
end

