function Stitched = stitch(A,B)
%Stitches structures A and B with the same fields together, following [A;B]

NamesOfFields = fieldnames(A);%Assume same for B:

for i = 1:length(NamesOfFields)

    Stitched.(NamesOfFields{i}) = [A.(NamesOfFields{i});B.(NamesOfFields{i})];
    
end

end