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
function [new_density] = diffuse(N,bound_for_XorY,new_density,starting_density,diff_const,dt)
    a = dt*diff_const*N*N;
    
    for k = 1:20
        for i = 2:N
            for j = 2:N
               new_density(i,j) = (starting_density(i,j) +a*(new_density(i-1,j) + new_density(i+1,j) + new_density(i,j-1) + new_density(i,j+1)))/(1+4*a);
            end
        end
        set_bnd(N,bound_for_XorY,new_density);
    end
end

%this code makes the given density field u follow a velocity field. Here we
%try and find what particle in the PREVIOUS step that will end up in the
%exact middle in the current step. That is why we will use 2 different
%containers. So basically simple linear backtracing
function [density] = advect(N, bound_for_XorY, density, velocity, dt)
    dt0 = dt*N;
    for i = 2:N
        for j = 2:N
           x = i -dt0*density(i,j);
           y = j -dt0*velocity(i,j);
           if (x < 0.5) %checks the boundry conditions, we cant be outside the area
              x = 0.5; 
           end
           if (x > N+0.5)
              x = N+0.5;
           end
           i0 = round(x); %this rounds up, be careful code in paper i0 = (int)x
           i1 = i0+1;
           
           if(y < 0.5)
              y = 0.5; 
           end
           if( y > N+0.5)
            y = N+0.5;
           end
           j0 = round(y); %this rounds up, be careful code in paper j0 = (int)y
           j1 = j0+1;
           
           s1 = x - i0;
           s0 = 1 - s1;
           t1 = y-j0;
           t0 = 1-t1;
           density(i,j) = s0*(t0*d0(i0,j0) +t1*d0(i0,j1)) + s1*(t0*d0(i1,j0) + t1*d0(i1,j1));
        end
    end
    set_bnd(N,bound_for_XorY,density);
end


%this function adds the dense part into 1 convenient step. Here the source
%denseties is contained in x0.
function [output_density] = dense_step(N, output_density, source_densities, density, velocity, diff, dt)
    add_source(N, output_density, source_densities, dt);
    %SWAP(x0,x) so for the difuse function swap the places of x and x0
    diffuse(N, 0, output_density, source_densities, diff, dt); %not done here
    %SWAP(x0,x) so for the advect function use the normal X and X0
    advect(N, 0, output_density, source_densities, density, velocity, dt); %as it should be
end

function [] = vel_step(N, output_density, output_velocity, force_density, force_velocity, visc, dt)
    add_source(N, output_density, force_density, dt);
    add_source(N, output_velocity, force_velocity, dt);
    
    %SWAP(u0,u)
    diffuse(N, 1, output_density, force_density, visc, dt); %have not swapped here =/
    %SWAP(v0,v)
    diffuse(N, 2, output_velocity, force_velocity, visc, dt); %have not swapped here =/
    project(N, output_density, output_velocity, force_density, force_velocity); %still not swapped
    %SWAP(u0, u)
    %SWAP(v0, v)
    advect( N, 1, output_density, force_density, force_density, force_velocity, dt); %as it should be
    advect(N, 2, output_velocity, force_velocity, force_density, force_velocity, dt); %as it should be
    project(N, output_density, output_velocity, force_density, force_velocity);
end

function [output_density, output_velocity] = project(N, output_density, output_velocity, project, div)
    h = 1.0/N;
    
    for i = 1:N
        for j = 1:N
           div(i,j) = -0.5*h*(output_density(i+1,j) -output_density(i-1,j) + output_velocity(i,j+1) - output_velocity(i,j-1));
           project(i,j) = 0;
        end
    end
    set_bnd(N,0,div);
    set_bnd(N,0,project);
    
    for k = 1:20
       for i = 1:N
          for j = 1:N
             project(i,j) = (div(i,j) + project(i-1,j) + project(i+1,j) +project(i,j-1) + project(i,j+1))/4; 
          end
       end
       set_bnd(N,0,project);
    end
    
    for i = 1:N
       for j = 1:N
          output_density(i,j) = output_density(i,j) - 0.5*(project(i+1,j) - project(i-1,j))/h;
          output_velocity(i,j) = output_velocity(i,j) - 0.5*(project(i,j+1) - project(i,j-1))/h;
       end
    end
    set_bnd(N,1,output_density);
    set_bnd(N,2,output_velocity);
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