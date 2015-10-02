function [ output_args ] = Navier()
%NAVIER Summary of this function goes here
%   Detailed explanation goes here

 N = 50;
 force_x = eye(N+2);  % 7x7 matrix
 force_y = eye(N+2,N+2); %7x7 matrix
 start_force_x = zeros(N+2,N+2); %7x7 matrix
 start_force_y = zeros(N+2,N+2); %7x7 matrix
 visc = 0.000099; %SI Units Pascal
 dt = 0.1;
 diff = 10;     %diffusion rate, constant ish
 
output_density = eye(N+2,N+2);
dens_prev = eye(N+2, N+2);
for i=1:20
    [force_x, force_y] = vel_step(N, force_x,force_y, start_force_x, start_force_y, visc, dt);
    output_density = dense_step(N, output_density, dens_prev, force_x, force_y, diff,dt);
    start_force_x = force_x;
    start_force_y = force_y;
    dens_prev = output_density;
end
imshow(force_x)
end
% Adding the source s to the density or veolcity x. We will have 1 box as
% the boundery so keep that in mind for the rest of this file. So the
% boundry boxes is all the i = 1 and i = N+1. Source is provided by the
% user. X here can be density or velocity or anything else
function[x] = add_source( N, x, source, dt)
    for i=1:(N+1)
        for j=1:(N+1)
            x(i,j) = x(i,j) + source(i,j)*dt;
        end
    end
end

%the diffuse term is smearing our water out. In this particular solution
%they take all the neighbours flux and remove/add it to the current node.
%The naive way only works when the diffuse is small. This implementation
%takes into concideration of the diffuse is large. They solve for the next
%timestep using Gauss-Seidel method to solve the equation system. The
%matrix is sparse so we can do this
function [output] = diffuse(N,bound_for_XorY,output,starting,diff_const,dt)
    a = dt*diff_const*N*N; %Difusion rate
    
    for k = 1:20
        for i = 2:N 
            for j = 2:N
               output(i,j) = (starting(i,j) +a*(output(i-1,j) + output(i+1,j) + output(i,j-1) + output(i,j+1)))/(1+4*a);
            end
        end
        output = set_bnd(N,bound_for_XorY,output);
    end
end

%this code makes the given density field u follow a velocity field. Here we
%try and find what particle in the PREVIOUS step that will end up in the
%exact middle in the current step. That is why we will use 2 different
%containers. So basically simple linear backtracing
function [output] = advect(N, b, output, starting, force_x, force_y, dt)
   dt0 = dt*N;
   for i = 2:N
      for j = 2:N
         x = i - dt0*force_x(i,j);
         y = j - dt0*force_y(i,j);
         
         if(x < 0.5)
            x = 0.5;
         elseif (x > N+0.5)
             x = N + 0.5;
         end
                        
         if(y < 0.5)
            y = 0.5; 
         elseif (y > N + 0.5)
             y = N + 0.5;
         end
         
         i0 = round(x); % i0 = (int) x
         i1 = i0 +1;
         j0 = round(y); % j0 = (int) y
         j1 = j0 + 1;
         
         s1 = x - i0;
         s0 = 1 - s1; 
         t1 = y - j0;
         t0 = 1 - t1;
         
         output(i,j) = s0*(  t0*starting(i0,j0) + t1*starting(i0,j1)  ) + s1*(  t0*starting(i1,j0) + t1*starting(i1,j1)  );
      end
   end
   
   output = set_bnd(N, b, output);
end


%this function adds the dense part into 1 convenient step. Here the source
%denseties is contained in x0.
function [output] = dense_step(N, output, source, force_x, force_y, diff, dt)
    output = add_source(N, output, source, dt);
    %SWAP(output,source)
    output = diffuse(N, 0, output, source, diff, dt); %not done here
    %SWAP(output,source) so for the advect function use the normal X and X0
    output = advect(N, 0, output, source, force_x, force_y, dt); %as it should be
end

function [force_x, force_y] = vel_step(N, force_x, force_y, start_force_x, start_force_y, visc, dt)
    force_x = add_source(N, force_x, start_force_x, dt);
    force_y = add_source(N, force_y, start_force_y, dt);
    
    %SWAP(density,force_density)
    force_x = diffuse(N, 1, force_x, start_force_x, visc, dt); %have not swapped here =/
    %SWAP(force_velocity,velocity)
    force_y = diffuse(N, 2, force_y, start_force_y, visc, dt); %have not swapped here =/
    [force_x, force_y] = project(N, force_x, force_y, start_force_x, start_force_y); %still not swapped
    %SWAP(force_density, density)
    %SWAP(force_velocity, velocity)
    force_x = advect( N, 1, force_x, start_force_x, start_force_x, start_force_y, dt); %as it should be
    force_y = advect(N, 2, force_y, start_force_y, start_force_x, start_force_y, dt); %as it should be
    [force_x, force_y] = project(N, force_x, force_y, start_force_x, start_force_y);
end

function [force_x, force_y] = project(N, force_x, force_y, preassure, div)
    h = 1.0/N;
    
    for i = 2:N
        for j = 2:N
           div(i,j) = -0.5*h*(force_x(i+1,j) -force_x(i-1,j) + force_y(i,j+1) - force_y(i,j-1));
           preassure(i,j) = 0;
        end
    end
    div = set_bnd(N,0,div);
    preassure = set_bnd(N,0,preassure);
    
    for k = 1:20  %Jacobi Method solving the matrix
       for i = 2:N
          for j = 2:N
             preassure(i,j) = (div(i,j) + preassure(i-1,j) + preassure(i+1,j) +preassure(i,j-1) + preassure(i,j+1))/4; 
          end
       end
       preassure = set_bnd(N,0,preassure);
    end
    
    for i = 2:N
       for j = 2:N
          force_x(i,j) = force_x(i,j) - 0.5*(preassure(i+1,j) - preassure(i-1,j))/h;  %dunno what this does, only add to the sides not up and down
          force_y(i,j) = force_y(i,j) - 0.5*(preassure(i,j+1) - preassure(i,j-1))/h; % same as above
       end
    end
    force_x = set_bnd(N,1,force_x);
    force_y = set_bnd(N,2,force_y);
end

function [x] = set_bnd(N, bound_for_XorY, x)

    for i= 1:N
       if( bound_for_XorY == 1)
          x(1,i) = -x(1,1);
          x(N+1,i) = -x(N,i);
       else
          x(1,i) = x(1,1);
          x(N+1,i) = x(N,1);
       end
       if( bound_for_XorY == 2)
          x(i,1) = -x(i,1);
          x(i,N+1) = -x(i,N);
       else
           x(i,1) = x(i,1);
           x(i,N+1) = x(i,N);
       end
       
       x(1,1) = 0.5*(x(1,1) + x(1,1));
       x(1,N+1) = 0.5*(x(1,N+1) + x(1,N));
       x(N+1,1) = 0.5*(x(N,1) + x(N+1,1));
       x(N+1,N+1) = 0.5*(x(N,N+1) + x(N+1,N));
       
    end
end