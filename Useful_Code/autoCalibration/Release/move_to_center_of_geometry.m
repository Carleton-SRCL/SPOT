function [x_centered, y_centered, x_shift, y_shift] = move_to_center_of_geometry(x, y)
    % Calculate the center of geometry
    x_shift = mean(x);
    y_shift = mean(y);
    
    % Move data to be centered at the center of geometry
    x_centered = x - x_shift;
    y_centered = y - y_shift;
end
