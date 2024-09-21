function [a, b, r, x_correction, y_correction] = fit_circle_and_correct(x, y)
    % Formulate the linear system
    A = [2*x, 2*y, ones(length(x), 1)];
    b = x.^2 + y.^2;
    
    % Solve the linear system using least squares
    theta = A \ b;
    
    % Extract the circle parameters
    a = theta(1);
    b = theta(2);
    c = theta(3);
    r = sqrt(a^2 + b^2 + c);
    
    % Calculate the correction to move measurements to CG
    x_correction = a - mean(x);
    y_correction = b - mean(y);
end
