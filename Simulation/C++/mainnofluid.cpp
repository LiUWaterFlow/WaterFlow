#include <iostream> //output
#include <algorithm> //for swap
#include <vector>

#include "fluidsolver.h"
#include "../../TSBB11/src/common/glm/glm.hpp"

const int N=10;
const int size=(N+2)*(N+2)*(N+2);

void set_bnd( int N, int b, float*** x) 
{
	int i,j;

	for(i =1; i<=N; i++)
	{
		for(j=1;j<=N;j++)
		{
			//b = 1 means forces in x-axis
			x[0][i][j] = 		(b == 1? -x[1][i][j] : x[1][i][j]);
			x[N+1][i][j] = 		(b == 1? -x[N][i][j] : x[N][i][j]);
			//b == 2 means forces in y-axis
			x[i][0][j] = 		(b == 2? -x[i][1][j] : x[i][1][j]);
			x[i][N+1][j] =		(b == 2? -x[i][N][j] : x[i][N][j]);
			//b == 3 means forces in z-axis
			x[i][j][0] = 		(b == 3? -x[i][j][1] : x[i][j][1]);
			x[i][j][N+1] =		(b == 3? -x[i][j][N] : x[i][j][N]);
		}
	}

	for(i=1; i<=N; i++)
	{
		x[0][0][i] = 		0.5f*(x[1][0][i] + x[0][1][i]);
		x[0][N+1][i] =	 	0.5f*(x[1][N+1][i] + x[0][N][i]);
		x[N+1][0][i] = 		0.5f*(x[N+1][1][i] + x[N][0][i]);
		x[N+1][N+1][i] = 		0.5f*(x[N][N+1][i] + x[N+1][N][i]);

		x[0][i][0] = 		0.5f*(x[1][i][0] + x[0][i][1]);
		x[N+1][i][0] = 		0.5f*(x[N][i][0] + x[N+1][i][1]);
		x[0][i][N+1] =	 	0.5f*(x[1][i][N+1] + x[0][i][N]);
		x[N+1][i][N+1] = 		0.5f*(x[N][i][N+1] + x[N+1][i][N]);	

		x[i][0][0] = 		0.5f*(x[i][1][0] + x[i][0][1]);
		x[i][N+1][0] = 		0.5f*(x[i][N][0] + x[i][N+1][1]);
		x[i][0][N+1] = 		0.5f*(x[i][1][N+1] + x[i][0][N]);
		x[i][N+1][N+1] = 		0.5f*(x[i][N][N+1] + x[i][N+1][N]);
	}
	
	x[0][0][0] = 0.3333f*(x[1][0][0] + x[0][1][0] + x[0][0][1]);
	x[0][N+1][0] = 0.3333f*(x[1][N+1][0] + x[0][N][0] + x[0][N+1][1]);

	x[N+1][0][0] = 0.3333f*(x[N][0][0] + x[N+1][1][0] + x[N+1][0][1]);
	x[N+1][N+1][0] = 0.3333f*(x[N][N+1][0] + x[N+1][N][0] + x[N+1][N+1][1]);

	x[0][0][N+1] = 0.3333f*(x[1][0][N+1] + x[0][1][N+1] + x[0][0][N]);
	x[0][N+1][N+1] = 0.3333f*(x[1][N+1][N+1] + x[0][N][N+1] + x[0][N+1][N]);

	x[N+1][0][N+1] = 0.3333f*(x[N][0][N+1] + x[N+1][1][N+1] + x[N+1][0][N]);
	x[N+1][N+1][N+1] = 0.3333f*(x[N][N+1][N+1] + x[N+1][N][N+1] + x[N+1][N+1][N]);

	
}


void add_source (int N, float ***x, float ***s, float dt) //fungerar som den ska yey
{
	int i,j,k;
	for (i=0;i<(N+2);i++)
	{
		for (j=0;j<(N+2);j++)
		{
			for (k=0;k<(N+2);k++)
			{
				x[i][j][k] += dt*s[i][j][k];
			}
		}
	}
}

void diffuse ( int N, int b, float ***x, float ***x0, float diff, float dt) //fungerar som den ska (tror vi) yey
{
	int iter, i,j,k;
	float a=dt*diff*N*N*N;

	for  (iter=0;iter<20;iter++)
	{
		for (i=1;i<=N;i++)
		{
			for (j=1;j<=N;j++)
			{
				for (k=1;k<=N;k++)
				{
				x[i][j][k] = (x0[i][j][k] + a*(
				x[i-1][j][k] + x[i+1][j][k] +
				x[i][j-1][k] + x[i][j+1][k] + 
				x[i][j][k-1] + x[i][j][k+1])  )/(1+6*a);
				}
			}
		}
		set_bnd (N,b,x);
	}

}

//Working we think it does that
void advect (int N, int b, float ***d, float ***d0, float ***u, float ***v, float ***w, float dt)
{
	int i,j,k,i0,j0,k0,i1,j1,k1;
	float x,y,z,s0,t0,q0,s1,t1,q1,dt0;

	dt0 = dt*N;
	for ( i=1;i<=N;i++)
	{
		for (j=1;j<=N;j++)
		{
			for (k=1;k<=N;k++)
			{
				x = i-dt0*u[i][j][k];
				y = j-dt0*v[i][j][k];
				z = k-dt0*w[i][j][k];

				if (x<0.5) 	{ x=0.5f; }
				if (x>N+0.5) 	{ x=(float)N+0.5f; }
				if (y<0.5) 	{ y=0.5f; }
				if (y>N+0.5)	{ y=(float)N+0.5f; }
				if (z<0.5) 	{ z=0.5f; }
				if (z>N+0.5)	{ z=(float)N+0.5f; }
			
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
			
				d[i][j][k] =	q0*(s0*(t0*d0[i0][j0][k0] + t1*d0[i0][j1][k0]) +
								s1*(t0*d0[i1][j0][k0] + t1*d0[i1][j1][k0])) +
								q1*(s0*(t0*d0[i0][j0][k1] + t1*d0[i0][j1][k1]) +
								s1*(t0*d0[i1][j0][k1] + t1*d0[i1][j1][k1])); //could look nicer
			}
		}
	}
	set_bnd (N,b,d);
}

void project (int N, float ***u, float ***v, float ***w, float ***p, float ***div)
{
	int i,j,k, iter;
	float h;

	h = 1.0f/(float)N;
	for (i=1;i<=N;i++)
	{
		for (j=1;j<=N;j++)
		{
			for(k = 1; k <= N; k++)
			{
				div[i][j][k] = -0.5f*h*(u[i+1][j][k]-u[i-1][j][k]+
								v[i][j+1][k]-v[i][j-1][k] +
								w[i][j][k+1]-w[i][j][k-1]); 
				p[i][j][k]=0;
			}		
		}
	}
	set_bnd (N,0,div); set_bnd (N,0,p);

	for (iter=0;iter<20;iter++)
	{
		for (i=1;i<=N;i++)
		{
			for (j=1;j<=N;j++)
			{
				for(k=1;k<=N;k++)
				{
					p[i][j][k] = (div[i][j][k] + p[i-1][j][k] + p[i+1][j][k] +
								p[i][j-1][k] + p[i][j+1][k] +
								p[i][j][k-1] + p[i][j][k+1])/6;
				}
			}
		}
		 set_bnd(N, 0,p);	
	}

	for( i=1; i<=N; i++)
	{
		for (j=1; j<= N; j++)
		{
			for(k=1;k<=N;k++)
			{
				u[i][j][k] -= 0.5f*(p[i+1][j][k] - p[i-1][j][k])/h;
				v[i][j][k] -= 0.5f*(p[i][j+1][k] - p[i][j-1][k])/h;
				w[i][j][k] -= 0.5f*(p[i][j][k+1] - p[i][j][k-1])/h;
			}
		}
	}

	set_bnd(N,1,u);
	set_bnd(N,2,v);
	set_bnd(N,3,w);
}	

void dens_step (int N, float ***x, float ***x0, float ***u, float ***v, float ***w, float diff, float dt)
{
	add_source (N,x,x0,dt);
	std::swap(x0, x); diffuse (N,0,x,x0,diff,dt);
	std::swap(x0, x); advect (N,0,x,x0,u,v,w,dt);
}

void vel_step (int N, float ***u, float ***v, float ***w, float ***u0, float ***v0, float ***w0, float visc, float dt)
{
	add_source (N,u,u0,dt);
	add_source (N,v,v0,dt);
	add_source (N,w,w0,dt);

	std::swap(u0,u); diffuse (N,1,u,u0,visc,dt);
	std::swap(v0,v); diffuse (N,2,v,v0,visc,dt);
	std::swap(w0,w); diffuse (N,3,w,w0,visc,dt);

	project (N,u,v,w,u0,v0); //still swaped

	std::swap(u0,u);
	std::swap(v0,v);
	std::swap(w0,w);

	advect (N,1,u,u0,u0,v0,w0,dt);
	advect (N,2,v,v0,u0,v0,w0,dt);
	advect (N,3,w,w0,u0,v0,w0,dt);
	project (N,u,v,w,u0,v0);
}

float sumArray(int N, float*** v)
{
	float sum = 0;
	for(int z = 1; z <= N; z++)
	{
		for(int y = 1; y <= N; y++)
		{
			for(int x = 1; x <= N; x++)
			{
				sum += v[x][y][z];
			}
		}
	}
	return sum;
}

void print(int N, float*** v)
{
	for(int z = 1; z <= N; z++)
	{
		std::cout << "z: " << z << "\n";
		for(int y = 1; y <= N; y++)
		{
			for(int x = 1; x <= N; x++)
			{
				std::cout << v[x][y][z] << " ";
			}
			std::cout << "\n";
		}
		std::cout << "\n\n";
	}
	std::cout << "Sum is: " << sumArray(N,v) << "\n";
	std::cout << std::endl;
}


void zeroArray(const int N, float*** x)
{
	for(int i = 0; i <= N+1; i++)
	{
		for(int j = 0; j <= N+1; j++)
		{
			for(int k = 0; k <= N+1; k++)
			{
				x[i][j][k] = 0.0f;
			}
		}
	}
}



int main()
{
	int N = 10;
	float visc = 1;
	float dt = 0.1f;
	//const int size = (N+2)*(N+2)*(N+2);
	float diff = 1;

	float*** dens = new float**[N+2];
	for (int i = 0; i < N+2; i++){
		dens[i] = new float*[N+2];
		for (int j = 0; j < N+2; j++){
			dens[i][j] = new float[N+2];
		}
	}

	float*** dens_prev = new float**[N+2];
	for (int i = 0; i < N+2; i++){
		dens_prev[i] = new float*[N+2];
		for (int j = 0; j < N+2; j++){
			dens_prev[i][j] = new float[N+2];
		}
	}

	float*** s = new float**[N+2];
	for (int i = 0; i < N+2; i++){
		s[i] = new float*[N+2];
		for (int j = 0; j < N+2; j++){
			s[i][j] = new float[N+2];
		}
	}

	float*** u = new float**[N+2];
	for (int i = 0; i < N+2; i++){
		u[i] = new float*[N+2];
		for (int j = 0; j < N+2; j++){
			u[i][j] = new float[N+2];
		}
	}

	float*** v = new float**[N+2];
	for (int i = 0; i < N+2; i++){
		v[i] = new float*[N+2];
		for (int j = 0; j < N+2; j++){
			v[i][j] = new float[N+2];
		}
	}

	float*** w = new float**[N+2];
	for (int i = 0; i < N+2; i++){
		w[i] = new float*[N+2];
		for (int j = 0; j < N+2; j++){
			w[i][j] = new float[N+2];
		}
	}

	float*** u_prev = new float**[N+2];
	for (int i = 0; i < N+2; i++){
		u_prev[i] = new float*[N+2];
		for (int j = 0; j < N+2; j++){
			u_prev[i][j] = new float[N+2];
		}
	}
	float*** v_prev = new float**[N+2];
	for (int i = 0; i < N+2; i++){
		v_prev[i] = new float*[N+2];
		for (int j = 0; j < N+2; j++){
			v_prev[i][j] = new float[N+2];
		}
	}

	float*** w_prev = new float**[N+2];
	for (int i = 0; i < N+2; i++){
		w_prev[i] = new float*[N+2];
		for (int j = 0; j < N+2; j++){
			w_prev[i][j] = new float[N+2];
		}
	}

	Voxel*** vox = new Voxel**[N+2];
	for (int i = 0; i < N+2; i++){
		vox[i] = new Voxel*[N+2];
		for (int j = 0; j < N+2; j++){
			vox[i][j] = new Voxel[N+2];
		}
	}

	zeroArray(N,dens);
	zeroArray(N,dens_prev);
	zeroArray(N,s);
	zeroArray(N,u);
	zeroArray(N,v);
	zeroArray(N,w);
	zeroArray(N,u_prev);
	zeroArray(N,v_prev);
	zeroArray(N,w_prev);

	dens_prev[2][2][2] = 10.0f;

	for(int i = 0; i < (N+2); i++) { for(int j = 0; j < (N+2); j++) { for(int k = 0; k < (N+2); k++)
	{
		u[i][j][k] = 10.0f;
		v[i][j][k] = 10.0f;
		w[i][j][k] = 10.0f;
	} } }
	
	for(int i=0; i<= 20; i++)
	{
		vel_step(N,u,v,w,u_prev,v_prev,w_prev,visc,dt);
		dens_step (N, dens, dens_prev, u, v, w,diff,dt);
		
		print(N,dens);
		//print(N,w);
		
	}

	// deallocate
	for (int i = 0; i < (N+2); i++){
		for (int j =0; j < (N+2); j++){
			delete [] dens[i][j];
		}
		delete [] dens[i];
	}
	delete [] dens;

	for (int i = 0; i < (N+2); i++){
		for (int j =0; j < (N+2); j++){
			delete [] dens_prev[i][j];
		}
		delete [] dens_prev[i];
	}
	delete [] dens_prev;

	for (int i = 0; i < (N+2); i++){
		for (int j =0; j < (N+2); j++){
			delete [] s[i][j];
		}
		delete [] s[i];
	}
	delete [] s;

	for (int i = 0; i < (N+2); i++){
		for (int j =0; j < (N+2); j++){
			delete [] u[i][j];
		}
		delete [] u[i];
	}
	delete [] u;

	for (int i = 0; i < (N+2); i++){
		for (int j =0; j < (N+2); j++){
			delete [] v[i][j];
		}
		delete [] v[i];
	}
	delete [] v;

	for (int i = 0; i < (N+2); i++){
		for (int j =0; j < (N+2); j++){
			delete [] w[i][j];
		}
		delete [] w[i];
	}
	delete [] w;

	for (int i = 0; i < (N+2); i++){
		for (int j =0; j < (N+2); j++){
			delete [] u_prev[i][j];
		}
		delete [] u_prev[i];
	}
	delete [] u_prev;

	for (int i = 0; i < (N+2); i++){
		for (int j =0; j < (N+2); j++){
			delete [] v_prev[i][j];
		}
		delete [] v_prev[i];
	}
	delete [] v_prev;

	for (int i = 0; i < (N+2); i++){
		for (int j =0; j < (N+2); j++){
			delete [] w_prev[i][j];
		}
		delete [] w_prev[i];
	}
	delete [] w_prev;

for (int i = 0; i < (N+2); i++){
		for (int j =0; j < (N+2); j++){
			delete [] vox[i][j];
		}
		delete [] vox[i];
	}
	delete [] vox;

	return 0;
}
