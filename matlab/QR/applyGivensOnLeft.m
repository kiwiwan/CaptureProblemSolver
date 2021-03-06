% Copyright 2018 CNRS-AIST JRL, CNRS-UM LIRMM
%
% This file is part of CPS.
%
% CPS is free software: you can redistribute it and/or modify
% it under the terms of the GNU Lesser General Public License as published by
% the Free Software Foundation, either version 3 of the License, or
% (at your option) any later version.
%
% CPS is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU Lesser General Public License for more details.
%
% You should have received a copy of the GNU Lesser General Public License
% along with CPS.  If not, see <http://www.gnu.org/licenses/>.


% perform Y = G_k^T G_{k-1}^T... G_1^T X, where G_i is the Givens rotation 
% described by g(i) (see g2mat).
function Y = applyGivensOnLeft(X,g)
Y = X;
for i=1:length(g)
  gi = g(i);
  Y(gi.i+(0:1),:) = [gi.c -gi.s; gi.s gi.c]* Y(gi.i+(0:1),:);
end
end