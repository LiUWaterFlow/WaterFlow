#define IX(i,j,k) ((i)+(N+2)*(j)+(N+2)*(N+2)*(k))
#define SWAP(x0, x) {float *tmp=x0;x0=x;x=tmp;}

#include "iostream"

//haxx så det funkar för tillfället
//void *__gxx_personality_v0;

const int N=10;
const int size=(N+2)*(N+2)*(N+2);

//static float u[size], v[size], w[size], u_prev[size], v_prev[size], w_prev[size];
//static float dens[size], dens_prev[size];


/*voxels between corners not bounded correctly
 first and last slice not working correctly
Works otherwise
*/
void set_bnd( int N, int b, float* x) 
{
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





void add_source (int N, float *x, float *s, float dt) //fungerar som den ska yey
{
	int i, size=(N+2)*(N+2)*(N+2);
	for (i=0;i<size;i++)
	{
		x[i] += dt*s[i];
	}
}

void diffuse ( int N, int b, float *x, float *x0, float diff, float dt) //fungerar som den ska (tror vi) yey
{
	int iter, i,j,k;
	float a=dt*diff*N*N*N;

	for  (iter=0;iter<1;iter++)
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
		set_bnd (N,b,x);
	}

}

//Still need to be checked. Pref by hand
void advect (int N, int b, float *d, float *d0, float *u, float *v, float *w, float dt)
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
	set_bnd (N,b,d);
}

void project (int N, float *u, float *v, float *w, float *p, float *div)
{
	int i,j,k, iter;
	float h;

	h = 1.0/N;
	for (i=1;i<=N;i++)
	{
		for (j=1;j<=N;j++)
		{
			for(k = 0; i <= N; k++)
			{
				div[IX(i,j,k)] = -0.5*h*(u[IX(i+1,j,k)]-u[IX(i-1,j,k)]+
						v[IX(i,j+1,k)]-v[IX(i,j-1,k)] +
						w[IX(i,j,k+1)] - w[IX(i,j,k-1)]); 
				p[IX(i,j,k)]=0;
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
					p[IX(i,j,k)] = (div[IX(i,j,k)] + p[IX(i-1,j,k)] + p[IX(i+1,j,k)] +
							p[IX(i,j-1,k)] + p[IX(i,j+1,k)] +
							p[IX(i,j,k-1)] + p[IX(i,j,k+1)])/6;
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
				u[IX(i,j,k)] -= 0.5*(p[IX(i+1,j,k)] - p[IX(i-1,j,k)])/h;
				v[IX(i,j,k)] -= 0.5*(p[IX(i,j+1,k)] - p[IX(i,j-1,k)])/h;
				w[IX(i,j,k)] -= 0.5*(p[IX(i,j,k+1)] - p[IX(i,j,k-1)])/h;
			}
		}
	}
	set_bnd(N,1,u);
	set_bnd(N,2,v);
	set_bnd(N,3,w);
}	

void dens_step (int N, float *x, float *x0, float *u, float *v, float *w, float diff, float dt)
{
	add_source (N,x,x0,dt);
	SWAP (x0, x); diffuse (N,0,x,x0,diff,dt);
	SWAP (x0, x); advect (N,0,x,x0,u,v,w,dt);
}

void vel_step (int N, float *u, float *v, float *w, float *u0, float *v0, float *w0, float visc, float dt)
{
	add_source (N,u,u0,dt);
	add_source (N,v,v0,dt);
	add_source (N,w,w0,dt);

	SWAP (u0,u); diffuse (N,1,u,u0,visc,dt);
	SWAP (v0,v); diffuse (N,2,v,v0,visc,dt);
	SWAP (w0,w); diffuse (N,3,w,w0,visc,dt);

	project (N,u,v,w,u0,v0); //still swaped

	SWAP (u0,u);
	SWAP (v0,v);
	SWAP (w0,w);

	advect (N,1,u,u0,u0,v0,w0,dt);
	advect (N,2,v,v0,u0,v0,w0,dt);
	advect (N,3,w,w0,u0,v0,w0,dt);
	project (N,u,v,w,u0,v0);
}

void print(int N, float* v)
{
	for(int z = 0; z < N + 2; z++)
	{
		std::cout << "z: " << z << "\n";
		for(int y = 0; y < N + 2; y++)
		{
			for(int x = 0; x < N + 2; x++)
			{
				std::cout << v[x+(N+2)*y+(N+2)*(N+2)*z] << " ";
			}
			std::cout << "\n";
		}
		std::cout << "\n\n";
	}
	std::cout << std::endl;
}

void zeroArray(const int size, float* x)
{
	for(int i = 0; i < size; i++)
	{
		x[i] = 0;
	}
}


int main()
{
	int N = 10;
	const int size = (N+2)*(N+2)*(N+2);
	float* x = new float[size];
	float* s = new float[size];
	float* u = new float[size];
	float* v = new float[size];
	float* w = new float[size];
	float dt = 0.1;

	zeroArray(size,x);
	zeroArray(size,s);
	zeroArray(size,u);
	zeroArray(size,v);
	zeroArray(size,w);
	
	add_source (N, x, s, dt);
	

	for(int i = 0; i < (N+2); i++) { for(int j = 0; j < (N+2); j++) { for(int k = 0; k < (N+2); k++)
	{
		s[IX(i,5,5)] = 1;
		u[IX(i,j,k)] = 0.1;
	} } }
	s[IX(5,5,5)] = 0.1;
	x[IX(2,2,2)] = 0.002;
	//diffuse (N,1,x,s,1,1);

	advect (N, 1, x, s, u, v, w, dt);
	print(N,x);



	delete[] x;
	delete[] s;
	return 0;
}
