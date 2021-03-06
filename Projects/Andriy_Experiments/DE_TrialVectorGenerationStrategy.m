function [TrialVector] = DE_TrialVectorGenerationStrategy(MethodIndex,MemberIndex,BestIndex,dimensionality,CR,F,K,Pop,PopSize)
%DE_TrialVectorGenerationStrategy Takes relevant info, makes trial vector
%   INPUTS:
%       MethodIndex - which strategy to use, out of pool of 4
%       MemberIndex - index of the member considered within Pop
%       BestIndex - index of the best member of the population
%       dimensionality - number of dimensions to consider
%       CR - crossover ratio of the current member
%       F - Scaling factor 'F' used in mutation operation
%       K - Scaling factor 'K' used in DE/current-to-rand/1 mutation style
%       Pop - Current Value (best) for each member of the population
%       PopSize - integer size of the population
%   OUTPUT:
%       TrialVector - The Trial vector to be tested.


stratNum = 4;

MethodIndex = round(MethodIndex);%sanitize
if MethodIndex>4 || MethodIndex<0
    MethodIndex = ceil(rand*stratNum);
end

%Choose j, and r1-4 indices:
sizeR = 5;
j = ceil(rand*dimensionality);
r = zeros(1,sizeR);
counter = 1;

while (counter <= sizeR) 
%dumbest way I've ever seen to check: 1) if there's a duplicate in r with itself 
%2) if there's a duplicate in j with r.
%3) if counter is greater than r.
%4) if there's a duplicate in r with Best index.
keepLooping = true;
    while(keepLooping) %A valid value for r(i) has not been found
        r(counter) = ceil(rand*PopSize);%Choose a random index for r(counter)
        if (sum(sum(r(1:counter)==r(1:counter)'-eye(counter))) || sum(j==r) || sum(BestIndex == r))
            keepLooping = true;
        else
           counter = counter+1;
           keepLooping = false;
        end
    end
end

switch MethodIndex
    
    case 1 %DE/rand/1/bin:
        for i = 1:dimensionality
            CrossoverChance = rand;
            if CrossoverChance < CR || i == j
            TrialVector(i) = Pop(i,r(1)) + F*(Pop(i,r(2)) - Pop(i,r(3)));
            else
            TrialVector(i) = Pop(i,MemberIndex);
            end
        end
       
    case 2 %DE/rand-to-best/2/bin:
        for i = 1:dimensionality
            CrossoverChance = rand;
            if CrossoverChance < CR || i == j
            TrialVector(i) = Pop(i,MemberIndex) + F * (Pop(i,BestIndex) - Pop(i,MemberIndex)) + F * (Pop(i,r(1)) - Pop(i,r(2))) + F * (Pop(i,r(3)) - Pop(i,r(4)));
            else
            TrialVector(i) = Pop(i,MemberIndex);
            end
        end
        
    case 3 %DE/rand/2/bin:
        for i = 1:dimensionality
            CrossoverChance = rand;
            if CrossoverChance < CR || i == j
                TrialVector(i) = Pop(i,r(1)) + F * (Pop(i,r(2)) - Pop(i,r(3))) + F * (Pop(i,r(4)) - Pop(i,r(5)));
            else
                TrialVector(i) = Pop(i,MemberIndex);
            end
        end
        
    case 4 %DE/current-to-rand/1:
        TrialVector = Pop(:,MemberIndex) + K * (Pop(:,r(1)) - Pop(:,MemberIndex)) + F * (Pop(:,r(2)) - Pop(:,r(3)));
        
end
        
        
        

end

