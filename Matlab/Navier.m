function [ output_args ] = Navier( N, u, v, u_input, v_input, visc, dt, dens, dens_input, diffe )
%NAVIER Summary of this function goes here
%   Detailed explanation goes here
vel_step(N,u,v,u_input, v_input, visc, dt);
dens_step(N, dens, dens_input, u, v, diffe, dt);
draw_dens(N, dens);

end
% Adding the source s to the density or veolcity x. We will have 1 box as
% the boundery so keep that in mind for the rest of this file. So the
% boundry boxes is all the i = 1 and i = N+1. Source is provided by the
% user. X here can be density or velocity or anything else
function[] = add_source( N, x, source, dt)
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
        set_bnd(N,bound_for_XorY,output);
    end
end

%this code makes the given density field u follow a velocity field. Here we
%try and find what particle in the PREVIOUS step that will end up in the
%exact middle in the current step. That is why we will use 2 different
%containers. So basically simple linear backtracing
function [density] = advect(N, b, d, d0, u, v, dt)
   dt0 = dt*N;
   for i = 2:N
      for j = 2:N
         x = i - dt0*u(i,j);
         y = j - dt0*v(i,j);
         
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
         
         d(i,j) = s0*(  t0*d0(i0,j0) + t1*d0(i0,j1)  ) + s1(  t0*d0(i1,j0) + t1*d0(i1,j1)  );
      end
   end
   
   set_bnd(N, b, d);
end


%this function adds the dense part into 1 convenient step. Here the source
%denseties is contained in x0.
function [output] = dense_step(N, output, source, density, velocity, diff, dt)
    add_source(N, output, source, dt);
    %SWAP(output,source)
    diffuse(N, 0, output, source, diff, dt); %not done here
    %SWAP(output,source) so for the advect function use the normal X and X0
    advect(N, 0, output, source, density, velocity, dt); %as it should be
end

function [] = vel_step(N, density, velocity, force_density, force_velocity, visc, dt)
    add_source(N, density, force_density, dt);
    add_source(N, velocity, force_velocity, dt);
    
    %SWAP(density,force_density)
    diffuse(N, 1, density, force_density, visc, dt); %have not swapped here =/
    %SWAP(force_velocity,velocity)
    diffuse(N, 2, velocity, force_velocity, visc, dt); %have not swapped here =/
    project(N, density, velocity, force_density, force_velocity); %still not swapped
    %SWAP(force_density, density)
    %SWAP(force_velocity, velocity)
    advect( N, 1, density, force_density, force_density, force_velocity, dt); %as it should be
    advect(N, 2, velocity, force_velocity, force_density, force_velocity, dt); %as it should be
    project(N, density, velocity, force_density, force_velocity);
end

function [density, velocity] = project(N, density, velocity, preassure, div)
    h = 1.0/N;
    
    for i = 1:N
        for j = 1:N
           div(i,j) = -0.5*h*(density(i+1,j) -density(i-1,j) + velocity(i,j+1) - velocity(i,j-1));
           preassure(i,j) = 0;
        end
    end
    set_bnd(N,0,div);
    set_bnd(N,0,preassure);
    
    for k = 1:20
       for i = 1:N
          for j = 1:N
             preassure(i,j) = (div(i,j) + preassure(i-1,j) + preassure(i+1,j) +preassure(i,j-1) + preassure(i,j+1))/4; 
          end
       end
       set_bnd(N,0,preassure);
    end
    
    for i = 1:N
       for j = 1:N
          density(i,j) = density(i,j) - 0.5*(preassure(i+1,j) - preassure(i-1,j))/h;
          velocity(i,j) = velocity(i,j) - 0.5*(preassure(i,j+1) - preassure(i,j-1))/h;
       end
    end
    set_bnd(N,1,density);
    set_bnd(N,2,velocity);
end

function [] = set_bnd(N, bound_for_XorY, x)

    for i= 1:N
       if( bound_for_XorY == 1)
          x(0,i) = -x(0,1);
          x(N+1,i) = -x(N,i);
       else
          x(0,i) = x(0,1);
          x(N+1,i) = x(N,1);
       end
       if( bound_for_XorY == 2)
          x(i,0) = -x(i,1);
          x(i,N+1) = -x(i,N);
       else
           x(i,0) = x(i,1);
           x(i,N+1) = x(i,N);
       end
       
       x(0,0) = 0.5*(x(1,0) + x(0,1));
       x(0,N+1) = 0.5*(x(1,N+1) + x(0,N));
       x(N+1,0) = 0.5*(x(N,0) + x(N+1,1));
       x(N+1,N+1) = 0.5*(x(N,N+1) + x(N+1,N));
       
    end
end