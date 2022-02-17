%% PLOTTING THE OUTPUTS:

% Plot the output position in 3d?
figure
grid on
hold on
plot3(simOut.rC_T(:,1), simOut.rC_T(:,2), simOut.rC_T(:,3));
xlabel("x axis (m)")
ylabel("y axis (m)")
zlabel("z axis (m)")
