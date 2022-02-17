function p = return_equality_vec(A, Np, x0)

    % First, get the dimension of A and B matrices:
    height_A = size(A, 1);
    
    % Preallocate the size:
    p = zeros(Np*height_A, 1);
    
    % create the p-vector:
    p(1:numel(x0)) = A*x0;
    
end

