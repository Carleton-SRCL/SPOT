function error = drdtMinusVel(x)
global r1x r1y r2x r2y r3x r3y r4x r4y v1x v1y v2x v2y v3x v3y v4x v4y Theta Omega deltaT lastrcomx1 lastrcomy1 lastrcomx2 lastrcomy2 lastrcomx3 lastrcomy3 lastrcomx4 lastrcomy4

r1comx = x(1);
r1comy = x(2);
r2comx = x(3);
r2comy = x(4);
r3comx = x(5);
r3comy = x(6);
r4comx = x(7);
r4comy = x(8);

vcomx1 = v1x + cos(Theta)*Omega*r1comy + sin(Theta)*Omega*r1comx;
vcomy1 = v1y + sin(Theta)*Omega*r1comy - cos(Theta)*Omega*r1comx;
vcomx2 = v2x + cos(Theta)*Omega*r2comy + sin(Theta)*Omega*r2comx;
vcomy2 = v2y + sin(Theta)*Omega*r2comy - cos(Theta)*Omega*r2comx;
vcomx3 = v3x + cos(Theta)*Omega*r3comy + sin(Theta)*Omega*r3comx;
vcomy3 = v3y + sin(Theta)*Omega*r3comy - cos(Theta)*Omega*r3comx;
vcomx4 = v4x + cos(Theta)*Omega*r4comy + sin(Theta)*Omega*r4comx;
vcomy4 = v4y + sin(Theta)*Omega*r4comy - cos(Theta)*Omega*r4comx;


rcomx1 = r1x - cos(Theta)*r1comx + sin(Theta)*r1comy;
rcomy1 = r1y - sin(Theta)*r1comx - cos(Theta)*r1comy;
rcomx2 = r2x - cos(Theta)*r2comx + sin(Theta)*r2comy;
rcomy2 = r2y - sin(Theta)*r2comx - cos(Theta)*r2comy;
rcomx3 = r3x - cos(Theta)*r3comx + sin(Theta)*r3comy;
rcomy3 = r3y - sin(Theta)*r3comx - cos(Theta)*r3comy;
rcomx4 = r4x - cos(Theta)*r4comx + sin(Theta)*r4comy;
rcomy4 = r4y - sin(Theta)*r4comx - cos(Theta)*r4comy;

drcomx1 = (rcomx1-lastrcomx1)/deltaT;
drcomy1 = (rcomy1-lastrcomy1)/deltaT;
drcomx2 = (rcomx2-lastrcomx2)/deltaT;
drcomy2 = (rcomy2-lastrcomy2)/deltaT;
drcomx3 = (rcomx3-lastrcomx3)/deltaT;
drcomy3 = (rcomy3-lastrcomy3)/deltaT;
drcomx4 = (rcomx4-lastrcomx4)/deltaT;
drcomy4 = (rcomy4-lastrcomy4)/deltaT;
%keyboard
%mean([r1comx r2comx r3comx r4comx r1comy r2comy r3comy r4comy])
if abs(mean([r1comx r2comx r3comx r4comx r1comy r2comy r3comy r4comy])) > 100
    fprintf('Kicked Out\n');
    error = 10e5;
else
    error = abs(vcomx1 + vcomy1 + vcomx2 + vcomy2 + vcomx3 + vcomy3 + vcomx4 + vcomy4 - drcomx1 - drcomy1 - drcomx2 - drcomy2 - drcomx3 - drcomy3 - drcomx4 - drcomy4);
end