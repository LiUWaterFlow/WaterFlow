#include "fluidsolver.h"

#include "iostream"





void FluidSolver::diffuse_one_velocity(NeighbourVoxels* vox, float constantData)
{
	//constantData =  a=dt*diff*N*N*N;
	vox->CURRENT_MID_CENTER.prev_velocity = (vox->CURRENT_MID_CENTER.velocity +
						constantData*(vox->CURRENT_MID_LEFT.prev_velocity + vox->CURRENT_MID_RIGHT.prev_velocity +
						vox->CURRENT_TOP_CENTER.prev_velocity + vox->CURRENT_BOTTOM_CENTER.prev_velocity +
						vox->BACK_MID_CENTER.prev_velocity + vox->FRONT_MID_CENTER.prev_velocity)  )/(1+6*constantData);
};

void FluidSolver::diffuse_one_density(NeighbourVoxels* vox, float constantData)
{
	//constantData =  a=dt*diff*N*N*N;
	vox->CURRENT_MID_CENTER.prev_density = (vox->CURRENT_MID_CENTER.density +
						constantData*(vox->CURRENT_MID_LEFT.prev_density + vox->CURRENT_MID_RIGHT.prev_density +
						vox->CURRENT_TOP_CENTER.prev_density + vox->CURRENT_BOTTOM_CENTER.prev_density +
						vox->BACK_MID_CENTER.prev_density + vox->FRONT_MID_CENTER.prev_density)  )/(1+6*constantData);
};


void FluidSolver::diffuse_velocity(float dt)
{

	float someconstant = dt;  // 
	NeighbourVoxels* temp;
	//not including borders
	for (unsigned int x = 1; x < m_grid.XLength - 1; x++)
	{
		for (unsigned int y = 1; y < m_grid.YLength - 1; y++)
		{
			for(unsigned int z = 1; z < m_grid.ZLength - 1; z++)
			{
				temp = m_grid.getNeighbourVoxels(x,y,z);
				diffuse_one_velocity(temp,someconstant);
			}
		}
	}
	delete temp;
};

void FluidSolver::diffuse_density(float dt)
{

	float someconstant = dt;  // 
	NeighbourVoxels* temp;
	//not including borders
	for (unsigned int x = 1; x < m_grid.XLength - 1; x++)
	{
		for (unsigned int y = 1; y < m_grid.YLength - 1; y++)
		{
			for(unsigned int z = 1; z < m_grid.ZLength - 1; z++)
			{
				temp = m_grid.getNeighbourVoxels(x,y,z);
				diffuse_one_density(temp,someconstant);
			}
		}
	}
	delete temp;
};



//everything from here needs to be double checked


void FluidSolver::advect_one_velocity(vector<int> prev_grid_position, vector<float> point_position, NeighbourVoxels* currentVox, float constantData);
{
	int prev_x = prev_grid_position[0]
	int prev_y = prev_grid_position[1]
	int prev_z = prev_grid_position[2]

	origintemp = m_grid.getNeighbourVoxels(prev_x,prev_y,prev_z);

	float s1 = point_position[0]-prev_grid_position[0];
	float s0 = 1-s1;
	float t1 = point_position[1]-prev_grid_position[1];
	float t0 = 1-t1;
	float q1 = point_position[2]-prev_grid_position[2];
	float q0 = 1-q1;	

	currentVox->CURRENT_MID_CENTER.velocity = q0*(s0*(t0*origintemp->CURRENT_MID_CENTER.prev_velocity + t1*origintemp->CURRENT_BOTTOM_CENTER.prev_velocity) +
							s1*(t0*origintemp->CURRENT_MID_RIGHT.prev_velocity + t1*origintemp->CURRENT_BOTTOM_RIGHT.prev_velocity)) +
							q1*(s0*(t0*origintemp->FRONT_MID_CENTER.prev_velocity + t1*origintemp->FRONT_BOTTOM_CENTER.prev_velocity) +
							s1*(t0*origintemp->FRONT_MID_RIGHT.prev_velocity + t1*origintemp->FRONT_BOTTOM_RIGHT.prev_velocity));
};

void FluidSolver::advect_velocity(float dt)
{
	float someconstant = dt;
	NeighbourVoxels* temp;
	//not including borders
	for (unsigned int x = 1; x < m_grid.XLength - 1; x++)
	{
		for (unsigned int y = 1; y < m_grid.YLength - 1; y++)
		{
			for(unsigned int z = 1; z < m_grid.ZLength - 1; z++)
			{
				temp = m_grid.getNeighbourVoxels(x,y,z);

				std::vector<int> gridPosition = {x,y,z};
				std::vector<float> pointPosition(3,0.0f);
				std::vector<int> prev_gridPosition(3,0);

				pointPosition = gridPosition - someconstant*temp->CURRENT_MID_CENTER.velocity; //might be prev_velocity

				if (pointPosition[0]<0.5) 	{ pointPosition[0]=0.5f; }
				if (pointPosition[0]>N+0.5) 	{ pointPosition[0]=(float)N+0.5f; }
				if (pointPosition[1]<0.5) 	{ pointPosition[1]=0.5f; }
				if (pointPosition[1]>N+0.5)	{ pointPosition[1]=(float)N+0.5f; }
				if (pointPosition[2]<0.5) 	{ pointPosition[2]=0.5f; }
				if (pointPosition[2]>N+0.5)	{ pointPosition[2]=(float)N+0.5f; }
			
				prev_gridPosition[0] = (int)pointPosition[0];
				prev_gridPosition[1] = (int)pointPosition[1];
				prev_gridPosition[2] = (int)pointPosition[2];

				advect_one_velocity(prev_gridPosition,pointPosition,temp,someconstant);			
			}
		}
	}
	delete temp;		
};

void FluidSolver::advect_one_density(vector<int> prev_grid_position, vector<float> point_position, NeighbourVoxels* currentVox, float constantData);
{
	int prev_x = prev_grid_position[0]
	int prev_y = prev_grid_position[1]
	int prev_z = prev_grid_position[2]

	origintemp = m_grid.getNeighbourVoxels(prev_x,prev_y,prev_z);

	float s1 = point_position[0]-prev_grid_position[0];
	float s0 = 1-s1;
	float t1 = point_position[1]-prev_grid_position[1];
	float t0 = 1-t1;
	float q1 = point_position[2]-prev_grid_position[2];
	float q0 = 1-q1;	

	currentVox->CURRENT_MID_CENTER.density = q0*(s0*(t0*origintemp->CURRENT_MID_CENTER.prev_density + t1*origintemp->CURRENT_BOTTOM_CENTER.prev_density) +
							s1*(t0*origintemp->CURRENT_MID_RIGHT.prev_density + t1*origintemp->CURRENT_BOTTOM_RIGHT.prev_density)) +
							q1*(s0*(t0*origintemp->FRONT_MID_CENTER.prev_density + t1*origintemp->FRONT_BOTTOM_CENTER.prev_density) +
							s1*(t0*origintemp->FRONT_MID_RIGHT.prev_density + t1*origintemp->FRONT_BOTTOM_RIGHT.prev_density));
};

void FluidSolver::advect_density(float dt)
{
	float someconstant = dt;
	NeighbourVoxels* temp;
	//not including borders
	for (unsigned int x = 1; x < m_grid.XLength - 1; x++)
	{
		for (unsigned int y = 1; y < m_grid.YLength - 1; y++)
		{
			for(unsigned int z = 1; z < m_grid.ZLength - 1; z++)
			{
				temp = m_grid.getNeighbourVoxels(x,y,z);

				std::vector<int> gridPosition = {x,y,z};
				std::vector<float> pointPosition(3,0.0f);
				std::vector<int> prev_gridPosition(3,0);

				pointPosition = gridPosition - someconstant*temp->CURRENT_MID_CENTER.velocity; //velocity regardless of calculating density or velocity

				if (pointPosition[0]<0.5) 	{ pointPosition[0]=0.5f; }
				if (pointPosition[0]>N+0.5) 	{ pointPosition[0]=(float)N+0.5f; }
				if (pointPosition[1]<0.5) 	{ pointPosition[1]=0.5f; }
				if (pointPosition[1]>N+0.5)	{ pointPosition[1]=(float)N+0.5f; }
				if (pointPosition[2]<0.5) 	{ pointPosition[2]=0.5f; }
				if (pointPosition[2]>N+0.5)	{ pointPosition[2]=(float)N+0.5f; }
			
				prev_gridPosition[0] = (int)pointPosition[0];
				prev_gridPosition[1] = (int)pointPosition[1];
				prev_gridPosition[2] = (int)pointPosition[2];

				advect_one_density(prev_gridPosition,pointPosition,temp,someconstant);			
			}
		}
	}
	delete temp;		
};































































































































/*
fluidsolver::fluidsolver(int N)
{
	_size = N;
	_dt = 0.1f;
}

fluidsolver::fluidsolver(int height, int width, int depth) : _height(height), _width(width), _depth(depth)
{
	_size = height*width*depth;
	_dt = 0.01;
}

void fluidsolver::set_bnd(int b, float* x)
{
	int N = _size;
	int i,j;

	for(i =1; i<=N; i++)
	{
		for(j=1;j<=N;j++)
		{
			//b = 1 means forces in x-axis
			x[IX(0,i,j)] = 		(b == 1? -x[IX(1,i,j)] : x[IX(1,i,j)]);
			x[IX(N+1,i,j)] = 	(b == 1? -x[IX(N,i,j)] : x[IX(N,i,j)]);
			//b == 2 means forces in y-axis
			x[IX(i,0,j)] = 		(b == 2? -x[IX(i,1,j)] : x[IX(i,1,j)]);
			x[IX(i,N+1,j)] =	(b == 2? -x[IX(i,N,j)] : x[IX(i,N,j)]);
			//b == 3 means forces in z-axis
			x[IX(i,j,0)] = 		(b == 3? -x[IX(i,j,1)] : x[IX(i,j,1)]);
			x[IX(i,j,N+1)] =	(b == 3? -x[IX(i,j,N)] : x[IX(i,j,N)]);
		}
	}

	for(i=1; i<=N; i++)
	{
		x[IX(0,0,i)] = 		0.5*(x[IX(1,0,i)] + x[IX(0,1,i)]);
		x[IX(0,N+1,i)] = 	0.5*(x[IX(1,N+1,i)] + x[IX(0,N,i)]);
		x[IX(N+1,0,i)] = 	0.5*(x[IX(N+1,1,i)] + x[IX(N,0,i)]);
		x[IX(N+1,N+1,i)] = 	0.5*(x[IX(N,N+1,i)] + x[IX(N+1,N,i)]);

		x[IX(0,i,0)] = 		0.5*(x[IX(1,i,0)] + x[IX(0,i,1)]);
		x[IX(N+1,i,0)] = 	0.5*(x[IX(N,i,0)] + x[IX(N+1,i,1)]);
		x[IX(0,i,N+1)] = 	0.5*(x[IX(1,i,N+1)] + x[IX(0,i,N)]);
		x[IX(N+1,i,N+1)] = 	0.5*(x[IX(N,i,N+1)] + x[IX(N+1,i,N)]);	

		x[IX(i,0,0)] = 		0.5*(x[IX(i,1,0)] + x[IX(i,0,1)]);
		x[IX(i,N+1,0)] = 	0.5*(x[IX(i,N,0)] + x[IX(i,N+1,1)]);
		x[IX(i,0,N+1)] = 	0.5*(x[IX(i,1,N+1)] + x[IX(i,0,N)]);
		x[IX(i,N+1,N+1)] = 	0.5*(x[IX(i,N,N+1)] + x[IX(i,N+1,N)]);
	}

	//corner boundries
	x[IX(0, 0,0)] = 0.3333f*(x[IX(1,0,0)] + x[IX(0,1,0)] + x[IX(0,0,1)]);
	x[IX(0, N+1,0)] = 0.3333f*(x[IX(1,N+1,0)] + x[IX(0,N,0)] + x[IX(0,N+1,1)]);

	x[IX(N+1, 0,0)] = 0.3333f*(x[IX(N,0,0)] + x[IX(N+1,1,0)] + x[IX(N+1,0,1)]);
	x[IX(N+1, N+1,0)] = 0.3333f*(x[IX(N,N+1,0)] + x[IX(N+1,N,0)] + x[IX(N+1,N+1,1)]);

	x[IX(0, 0,N+1)] = 0.3333f*(x[IX(1,0,N+1)] + x[IX(0,1,N+1)] + x[IX(0,0,N)]);
	x[IX(0, N+1,N+1)] = 0.3333f*(x[IX(1,N+1,N+1)] + x[IX(0,N,N+1)] + x[IX(0,N+1,N)]);

	x[IX(N+1, 0,N+1)] = 0.3333f*(x[IX(N,0,N+1)] + x[IX(N+1,1,N+1)] + x[IX(N+1,0,N)]);
	x[IX(N+1, N+1,N+1)] = 0.3333f*(x[IX(N,N+1,N+1)] + x[IX(N+1,N,N+1)] + x[IX(N+1,N+1,N)]);

}

void fluidsolver::add_source(float *x, float *s)
{
	float dt = _dt;
	int i, size=(_size+2)*(_size+2)*(_size+2); //add to the borders too
	for (i=0;i<size;i++)
	{
		x[i] += dt*s[i];
	}
}

void fluidsolver::diffuse (int b, float *x, float *x0, float diff) //fungerar som den ska (tror vi) yey
{
	int N = _size;
	int iter, i,j,k;
	float a= _dt*diff*N*N*N;

	for  (iter=0;iter<20;iter++)
	{
		for (i=1;i<=N;i++)
		{
			for (j=1;j<=N;j++)
			{
				for (k=1;k<=N;k++)
				{
				x[IX(i,j,k)] = (x0[IX(i,j,k)] + a*(
				x[IX(i-1,j,k)] + x[IX(i+1,j,k)] +
				x[IX(i,j-1,k)] + x[IX(i,j+1,k)] + 
				x[IX(i,j,k-1)] + x[IX(i,j,k+1)])  )/(1+6*a);
				}
			}
		}
		set_bnd (b,x);
	}
}

void fluidsolver::advect (int b, float *d, float *d0, float *u, float *v, float *w)
{
	float dt = _dt;
	int N = _size;
	int i,j,k,i0,j0,k0,i1,j1,k1;
	float x,y,z,s0,t0,q0,s1,t1,q1,dt0;

	dt0 = dt*N;
	for ( i=1;i<=N;i++)
	{
		for (j=1;j<=N;j++)
		{
			for (k=1;k<=N;k++)
			{
				x = i-dt0*u[IX(i,j,k)];
				y = j-dt0*v[IX(i,j,k)];
				z = k-dt0*w[IX(i,j,k)];

				if (x<0.5) 	{ x=0.5; }
				if (x>N+0.5) 	{ x=N+0.5; }
				if (y<0.5) 	{ y=0.5; }
				if (y>N+0.5)	{ y=N+0.5; }
				if (z<0.5) 	{ z=0.5; }
				if (z>N+0.5)	{ z=N+0.5; }
			
				i0 = (int)x;
				i1 = i0 + 1;
				j0 = (int)y;
				j1 = j0 + 1;
				k0 = (int)z;
				k1 = k0 + 1;
			
				s1 = x-i0;
				s0 = 1-s1;
				t1 = y-j0;
				t0 = 1-t1;
				q1 = z-k0;
				q0 = 1-q1;
			
				d[IX(i,j,k)] =	q0*(s0*(t0*d0[IX(i0,j0,k0)] + t1*d0[IX(i0,j1,k0)]) +
							s1*(t0*d0[IX(i1,j0,k0)] + t1*d0[IX(i1,j1,k0)])) +
						q1*(s0*(t0*d0[IX(i0,j0,k1)] + t1*d0[IX(i0,j1,k1)]) +
							s1*(t0*d0[IX(i1,j0,k1)] + t1*d0[IX(i1,j1,k1)])); //could look nicer
			}
		}
	}
	set_bnd (b,d);
}

void fluidsolver::project (float *u, float *v, float *w, float *p, float *divergence)
{
	int N = _size;
	int i,j,k, iter;
	float h;

	h = 1.0/N;
	for (i=1;i<=N;i++)
	{
		for (j=1;j<=N;j++)
		{
			for(k = 1; k <= N; k++)
			{
				divergence[IX(i,j,k)] = -0.5*h*(u[IX(i+1,j,k)]-u[IX(i-1,j,k)]+
						v[IX(i,j+1,k)]-v[IX(i,j-1,k)] +
						w[IX(i,j,k+1)] - w[IX(i,j,k-1)]); 
				p[IX(i,j,k)]=0;
			}		
		}
	}
	set_bnd (0,divergence); set_bnd (0,p);

	for (iter=0;iter<20;iter++)
	{
		for (i=1;i<=N;i++)
		{
			for (j=1;j<=N;j++)
			{
				for(k=1;k<=N;k++)
				{
					p[IX(i,j,k)] = (divergence[IX(i,j,k)] + p[IX(i-1,j,k)] + p[IX(i+1,j,k)] +
							p[IX(i,j-1,k)] + p[IX(i,j+1,k)] +
							p[IX(i,j,k-1)] + p[IX(i,j,k+1)])/6;
				}
			}
		}
		 set_bnd(0,p);	
	}

	for( i=1; i<=N; i++)
	{
		for (j=1; j<= N; j++)
		{
			for(k=1;k<=N;k++)
			{
				u[IX(i,j,k)] -= 0.5*(p[IX(i+1,j,k)] - p[IX(i-1,j,k)])/h;
				v[IX(i,j,k)] -= 0.5*(p[IX(i,j+1,k)] - p[IX(i,j-1,k)])/h;
				w[IX(i,j,k)] -= 0.5*(p[IX(i,j,k+1)] - p[IX(i,j,k-1)])/h;
			}
		}
	}

	set_bnd(1,u);
	set_bnd(2,v);
	set_bnd(3,w);
}

void fluidsolver::dens_step (float *x, float *x0, float *u, float *v, float *w, float diff)
{
	add_source (x,x0);
	SWAP (x0, x); diffuse (0,x,x0,diff);
	SWAP (x0, x); advect (0,x,x0,u,v,w);
}

void fluidsolver::vel_step (float *u, float *v, float *w, float *u0, float *v0, float *w0, float visc)
{
	add_source (u,u0);
	add_source (v,v0);
	add_source (w,w0);

	SWAP (u0,u); diffuse (1,u,u0,visc);
	SWAP (v0,v); diffuse (2,v,v0,visc);
	SWAP (w0,w); diffuse (3,w,w0,visc);

	project (u,v,w,u0,v0); //still swaped

	SWAP (u0,u);
	SWAP (v0,v);
	SWAP (w0,w);

	advect (1,u,u0,u0,v0,w0);
	advect (2,v,v0,u0,v0,w0);
	advect (3,w,w0,u0,v0,w0);
	project (u,v,w,u0,v0);
}

void fluidsolver::print(float* v)
{
	int N = _size;
	for(int z = 0; z < N + 2; z++)
	{
		std::cout << "z: " << z << "\n";
		for(int y = 1; y < N + 1; y++)
		{
			for(int x = 1; x < N + 1; x++)
			{
				std::cout << v[x+(N+2)*y+(N+2)*(N+2)*z] << " ";
			}
			std::cout << "\n";
		}
		std::cout << "\n\n";
	}
}
*/
