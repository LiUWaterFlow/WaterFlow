function [ output ] = Navier()
%NAVIER Summary of this function goes here
%   Detailed explanation goes here

 N = 3;
 force_x = zeros(N+2);  %
 force_y = zeros(N+2,N+2); %
 start_force_x = zeros(N+2,N+2); %
 start_force_y = zeros(N+2,N+2); %
 visc =0.01; %SI Units Pascal
 dt = 0.1;  %time step size
 diffuse_rate = 0.01;     %diffusion rate, constant ish
 
output = zeros(N+2,N+2);
source = zeros(N+2, N+2);
source(3,3) = 1;
for k=1:(N+2)
    for o=1:(N+2)
       force_y(k,o) = -9.81; 
    end
end

%diffuse_step
disp('Add Source');
output = add_source ( N, output, source, dt )
disp('Diffuse source');
source = diffuse ( N, 0, source, output, diffuse_rate, dt )
disp('Advect Output');
output = advect ( N, 0, output, source, force_x, force_y, dt )

end

%{Adding sources to all grid cells
%}
function[x] = add_source( N, x, source, dt)
    for i=1:(N+2)
        for j=1:(N+2)
            x(i,j) = x(i,j) + source(i,j)*dt;
        end
    end
end

%{
% the diffuse term is smearing our water out. In this particular solution
%they take all the neighbours values and add it to the current node.
%They solve for the next timestep using Gauss-Seidel method. The matrix is
% sparse so we can do this.
%
% WARNING THIS NEEDS TO BE CHANGED FOR 3D. Shall we think about doing a
proper smear?

The (1+4*a) should be (1+6*a) for cubes
%}
function [output] = diffuse(N,bound_for_XorY,output,starting,diff_const,dt)
    a = dt*diff_const*(N+1)*(N+1); %Difusion rate
    
    %this part could be itsown function
    for k = 1:10
        for i = 2:(N+1) 
            for j = 2:(N+1)
               output(i,j) = (starting(i,j) +a*(output(i-1,j) + output(i+1,j) + output(i,j-1) + output(i,j+1)))/(1+4*a);
            end
        end
        output = set_bnd(N,bound_for_XorY,output);
    end
end

%{
%this code makes the given density field u follow a velocity field. Here we
%try and find what particle in the PREVIOUS step that will end up in the
%exact middle in the current step. That is why we will use 2 different
%containers. So basically simple linear backtracing
%}
function [output] = advect(N, bound_for_XorY, output, starting, force_x, force_y, dt)
   dt0 = dt*(N+1);
   for i = 2:(N+1)
      for j = 2:(N+1)
         x = i - dt0*force_x(i,j);
         y = j - dt0*force_y(i,j);
         
         if(x < 0.5) %clamping so we are not outside the grid
            x = 0.5;
         elseif (x > (N+1) + 0.5)
             x = (N+1) + 0.5;
         end
                        
         if(y < 0.5)
            y = 0.5; 
         elseif (y > (N+1) + 0.5)
             y = (N+1) + 0.5;
         end
         
         i0 = floor(x); % i0 = (int) x
         i1 = i0 +1;
         j0 = floor(y); % j0 = (int) y
         j1 = j0 + 1;
         
         s1 = x - i0;
         s0 = 1 - s1; 
         t1 = y - j0;
         t0 = 1 - t1;
         %below solves the laplacian
         output(i,j) = s0*(  t0*starting(i0,j0) + t1*starting(i0,j1)  ) + s1*(  t0*starting(i1,j0) + t1*starting(i1,j1)  );
      end
   end
   
   output = set_bnd(N, bound_for_XorY, output);
end

function [output] = dense_step(N, output, source, force_x, force_y, diff, dt)

    disp('add_source');
    output = add_source(N, output, source, dt);
    %SWAP(output,source)
    disp('diffuse');
    source = diffuse(N, 0, source, output, diff, dt); %NOTICE DATA SWAPED
    %SWAP(output,source)
    disp('advect');
    output = advect(N, 0, output, source, force_x, force_y, dt); %as it should be
end

function [force_x, force_y] = vel_step(N, force_x, force_y, start_force_x, start_force_y, visc, dt)
    force_x = add_source(N, force_x, start_force_x, dt);
    force_y = add_source(N, force_y, start_force_y, dt);
    
    %SWAP(force_x, start_force_x)
    force_x = diffuse(N, 1, start_force_x, force_x, visc, dt); %notice the input swap
    %SWAP(force_y,start_force_y)
    force_y = diffuse(N, 2, start_force_y, force_y, visc, dt); %notice input swap
    [force_x, force_y] = project(N, start_force_x, start_force_y, force_x, force_y); %notice input swap
    %SWAP(force_density, density)
    %SWAP(force_velocity, velocity)
    force_x = advect( N, 1, force_x, start_force_x, start_force_x, start_force_y, dt); %as it should be
    force_y = advect(N, 2, force_y, start_force_y, start_force_x, start_force_y, dt); %as it should be
    [force_x, force_y] = project(N, force_x, force_y, start_force_x, start_force_y);
end

%sends the preassure through the force field.
function [force_x, force_y] = project(N, force_x, force_y, preassure, div)
    h = 1.0/N;
    
    for i = 2:(N+1)
        for j = 2:(N+1)
           div(i,j) = -0.5*h*(force_x(i+1,j) -force_x(i-1,j) + force_y(i,j+1) - force_y(i,j-1));
           preassure(i,j) = 0;
        end
    end
    div = set_bnd(N,0,div);
    preassure = set_bnd(N,0,preassure);
    
    for k = 1:20  %Jacobi Method solving the matrix
       for i = 2:(N+1)
          for j = 2:(N+1)
             preassure(i,j) = (div(i,j) + preassure(i-1,j) + preassure(i+1,j) +preassure(i,j-1) + preassure(i,j+1))/4; 
          end
       end
       preassure = set_bnd(N,0,preassure);
    end
    
    for i = 2:(N+1)
       for j = 2:(N+1)
          force_x(i,j) = force_x(i,j) - 0.5*(preassure(i+1,j) - preassure(i-1,j))/h;  %dunno what this does, only add to the sides not up and down
          force_y(i,j) = force_y(i,j) - 0.5*(preassure(i,j+1) - preassure(i,j-1))/h; % same as above
       end
    end
    force_x = set_bnd(N,1,force_x);
    force_y = set_bnd(N,2,force_y);
end

function [x] = set_bnd(N, bound_XorY, x)

    for i= 2:(N+1)
       if( bound_XorY == 1)
          x(1,i) = -x(2,i);
          x(N+2,i) = -x(N+1,i);
       elseif (bound_XorY == 2)
          x(i,1) = -x(i,2);
          x(i,N+2) = -x(i,N+1);
       else
           x(1,i) = x(2,i);
           x(N+2,i) = x(N+1,i);
           x(i,1) = x(i,2);
           x(i,N+2) = x(i,N+1);
       end
       
       x(1,1) = 0.5*(x(2,1) + x(1,2)); %top left corner
       x(1,N+2) = 0.5*(x(2,N+2) + x(1,N+1)); %bottom left corner
       x(N+2,1) = 0.5*(x(N+1,1) + x(N+2,2)); %top right corner
       x(N+2,N+2) = 0.5*(x(N+1,N+2) + x(N+2,N+1)); %bottom right corner
    end
end