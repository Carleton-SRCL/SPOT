function [C] = return_equality_mat(A, B, Np)

    % First, get the dimension of A and B matrices:
    height_A = size(A, 1);
    width_B = size(B,2);
    
    AB = [A, B];
    width_AB = size(AB,2);
    
    ABI = [A, B, -eye(height_A)];
    width_ABI = size(ABI, 2);
    
    BI = [B, -eye(height_A)];
    width_BI = size(BI,2);
    
    % Preallocate the size:
    C = zeros(Np*height_A, Np*width_AB);
    
    % Loop through creating the dynamics matrix:
    iHeight = 1;
    iWidth = 1;
    
    for iStep = 1 : Np
        % Get the end of the height:
        iHeightEnd = iHeight + height_A - 1;
        if iStep == 1
            C(iHeight:iHeightEnd, iWidth:iWidth + width_BI - 1) = BI;
            iWidth = iWidth + width_B;
        else
           C(iHeight:iHeightEnd, iWidth:iWidth + width_ABI - 1) = ABI;
           iWidth = iWidth + width_BI;
        end
        % increment the height:
        iHeight = iHeightEnd + 1;
    end
    
end

