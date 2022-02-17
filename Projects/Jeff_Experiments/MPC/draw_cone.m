function [] = draw_cone(A_cone, b_cone, xlims, ylims, zlims, res)
% Just to test drawing the cone. It is probably backwards again...

figure

X = xlims(1):res:xlims(2);
Y = ylims(1):res:ylims(2);
Z = zlims(1):res:zlims(2);

hold on
grid on

    for x = X
        for y = Y
            for z = Z

                point = [x;y;z];
                
                if allPositive((b_cone - A_cone*point)') == 1 % It IS in the cone.
                    plot3(x, y, z, 'rx');
                end


            end
        end
    end
    
xlabel('x axis')
ylabel('y axis')
zlabel('z axis')
axis equal

end

function out = allPositive(in)

out = 1; % All are positive.

for i = in
    if i <= 0
        out = 0; % One is negative.
        return
    end
end

end