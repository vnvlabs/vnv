
Branch Status. 


Vector support is all there

Run the example 

mpirun -n 2 ./distributed 10 3 


That will run an example with a vector with 10elements on each proc and a matrix 
with 3x3 block on each proc.

Look at distributed.cpp to see an example



Also added some support in this branch for iteration points. These are chunks of code 
we can run a bunch of times to do things like UQ/SA and parameter searches. I am in the 
middle of developing a plugin for Dakota to work at those points. That is not working 
yet (should have been a different branch ) 





