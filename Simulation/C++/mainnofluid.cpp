//#define IX(i,j,k) ((i)+(N+2)*(j)+(N+2)*(N+2)*(k))

//new defines
#define IX(i) (i)
#define IY(j) (j)
#define IZ(k) (k)

#define SWAP(x0, x) {float ***tmp=x0;x0=x;x=tmp;}

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

void set_bnd( int N, int b, float*** x) 
{
	int i,j;

	for(i =1; i<=N; i++)
	{
		for(j=1;j<=N;j++)
		{
			//b = 1 means forces in x-axis
			x[IX(0)][IY(i)][IZ(j)] = 		(b == 1? -x[IX(1)][IY(i)][IZ(j)] : x[IX(1)][IY(i)][IZ(j)]);
			x[IX(N+1)][IY(i)][IZ(j)] = 		(b == 1? -x[IX(N)][IY(i)][IZ(j)] : x[IX(N)][IY(i)][IZ(j)]);
			//b == 2 means forces in y-axis
			x[IX(i)][IY(0)][IZ(j)] = 		(b == 2? -x[IX(i)][IY(1)][IZ(j)] : x[IX(i)][IY(1)][IZ(j)]);
			x[IX(i)][IY(N+1)][IZ(j)] =		(b == 2? -x[IX(i)][IY(N)][IZ(j)] : x[IX(i)][IY(N)][IZ(j)]);
			//b == 3 means forces in z-axis
			x[IX(i)][IY(j)][IZ(0)] = 		(b == 3? -x[IX(i)][IY(j)][IZ(1)] : x[IX(i)][IY(j)][IZ(1)]);
			x[IX(i)][IY(j)][IZ(N+1)] =		(b == 3? -x[IX(i)][IY(j)][IZ(N)] : x[IX(i)][IY(j)][IZ(N)]);
		}
	}

	for(i=1; i<=N; i++)
	{
		x[IX(0)][IY(0)][IZ(i)] = 		0.5*(x[IX(1)][IY(0)][IZ(i)] + x[IX(0)][IY(1)][IZ(i)]);
		x[IX(0)][IY(N+1)][IZ(i)] =	 	0.5*(x[IX(1)][IY(N+1)][IZ(i)] + x[IX(0)][IY(N)][IZ(i)]);
		x[IX(N+1)][IY(0)][IZ(i)] = 		0.5*(x[IX(N+1)][IY(1)][IZ(i)] + x[IX(N)][IY(0)][IZ(i)]);
		x[IX(N+1)][IY(N+1)][IZ(i)] = 		0.5*(x[IX(N)][IY(N+1)][IZ(i)] + x[IX(N+1)][IY(N)][IZ(i)]);

		x[IX(0)][IY(i)][IZ(0)] = 		0.5*(x[IX(1)][IY(i)][IZ(0)] + x[IX(0)][IY(i)][IZ(1)]);
		x[IX(N+1)][IY(i)][IZ(0)] = 		0.5*(x[IX(N)][IY(i)][IZ(0)] + x[IX(N+1)][IY(i)][IZ(1)]);
		x[IX(0)][IY(i)][IZ(N+1)] =	 	0.5*(x[IX(1)][IY(i)][IZ(N+1)] + x[IX(0)][IY(i)][IZ(N)]);
		x[IX(N+1)][IY(i)][IZ(N+1)] = 		0.5*(x[IX(N)][IY(i)][IZ(N+1)] + x[IX(N+1)][IY(i)][IZ(N)]);	

		x[IX(i)][IY(0)][IZ(0)] = 		0.5*(x[IX(i)][IY(1)][IZ(0)] + x[IX(i)][IY(0)][IZ(1)]);
		x[IX(i)][IY(N+1)][IZ(0)] = 		0.5*(x[IX(i)][IY(N)][IZ(0)] + x[IX(i)][IY(N+1)][IZ(1)]);
		x[IX(i)][IY(0)][IZ(N+1)] = 		0.5*(x[IX(i)][IY(1)][IZ(N+1)] + x[IX(i)][IY(0)][IZ(N)]);
		x[IX(i)][IY(N+1)][IZ(N+1)] = 		0.5*(x[IX(i)][IY(N)][IZ(N+1)] + x[IX(i)][IY(N+1)][IZ(N)]);
	}
	
	x[IX(0)][IY(0)][IZ(0)] = 0.3333f*(x[IX(1)][IY(0)][IZ(0)] + x[IX(0)][IY(1)][IZ(0)] + x[IX(0)][IY(0)][IZ(1)]);
	x[IX(0)][IY(N+1)][IZ(0)] = 0.3333f*(x[IX(1)][IY(N+1)][IZ(0)] + x[IX(0)][IY(N)][IZ(0)] + x[IX(0)][IY(N+1)][IZ(1)]);

	x[IX(N+1)][IY(0)][IZ(0)] = 0.3333f*(x[IX(N)][IY(0)][IZ(0)] + x[IX(N+1)][IY(1)][IZ(0)] + x[IX(N+1)][IY(0)][IZ(1)]);
	x[IX(N+1)][IY(N+1)][IZ(0)] = 0.3333f*(x[IX(N)][IY(N+1)][IZ(0)] + x[IX(N+1)][IY(N)][IZ(0)] + x[IX(N+1)][IY(N+1)][IZ(1)]);

	x[IX(0)][IY(0)][IZ(N+1)] = 0.3333f*(x[IX(1)][IY(0)][IZ(N+1)] + x[IX(0)][IY(1)][IZ(N+1)] + x[IX(0)][IY(0)][IZ(N)]);
	x[IX(0)][IY(N+1)][IZ(N+1)] = 0.3333f*(x[IX(1)][IY(N+1)][IZ(N+1)] + x[IX(0)][IY(N)][IZ(N+1)] + x[IX(0)][IY(N+1)][IZ(N)]);

	x[IX(N+1)][IY(0)][IZ(N+1)] = 0.3333f*(x[IX(N)][IY(0)][IZ(N+1)] + x[IX(N+1)][IY(1)][IZ(N+1)] + x[IX(N+1)][IY(0)][IZ(N)]);
	x[IX(N+1)][IY(N+1)][IZ(N+1)] = 0.3333f*(x[IX(N)][IY(N+1)][IZ(N+1)] + x[IX(N+1)][IY(N)][IZ(N+1)] + x[IX(N+1)][IY(N+1)][IZ(N)]);

	
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
				x[IX(i)][IY(j)][IZ(k)] = (x0[IX(i)][IY(j)][IZ(k)] + a*(
				x[IX(i-1)][IY(j)][IZ(k)] + x[IX(i+1)][IY(j)][IZ(k)] +
				x[IX(i)][IY(j-1)][IZ(k)] + x[IX(i)][IY(j+1)][IZ(k)] + 
				x[IX(i)][IY(j)][IZ(k-1)] + x[IX(i)][IY(j)][IZ(k+1)])  )/(1+6*a);
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
				x = i-dt0*u[IX(i)][IY(j)][IZ(k)];
				y = j-dt0*v[IX(i)][IY(j)][IZ(k)];
				z = k-dt0*w[IX(i)][IY(j)][IZ(k)];

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
			
				d[IX(i)][IY(j)][IZ(k)] =	q0*(s0*(t0*d0[IX(i0)][IY(j0)][IZ(k0)] + t1*d0[IX(i0)][IY(j1)][IZ(k0)]) +
								s1*(t0*d0[IX(i1)][IY(j0)][IZ(k0)] + t1*d0[IX(i1)][IY(j1)][IZ(k0)])) +
								q1*(s0*(t0*d0[IX(i0)][IY(j0)][IZ(k1)] + t1*d0[IX(i0)][IY(j1)][IZ(k1)]) +
								s1*(t0*d0[IX(i1)][IY(j0)][IZ(k1)] + t1*d0[IX(i1)][IY(j1)][IZ(k1)])); //could look nicer
			}
		}
	}
	set_bnd (N,b,d);
}

void project (int N, float ***u, float ***v, float ***w, float ***p, float ***div)
{
	int i,j,k, iter;
	float h;

	h = 1.0/N;
	for (i=1;i<=N;i++)
	{
		for (j=1;j<=N;j++)
		{
			for(k = 1; k <= N; k++)
			{
				div[IX(i)][IY(j)][IZ(k)] = -0.5*h*(u[IX(i+1)][IY(j)][IZ(k)]-u[IX(i-1)][IY(j)][IZ(k)]+
								v[IX(i)][IY(j+1)][IZ(k)]-v[IX(i)][IY(j-1)][IZ(k)] +
								w[IX(i)][IY(j)][IZ(k+1)]-w[IX(i)][IY(j)][IZ(k-1)]); 
				p[IX(i)][IY(j)][IZ(k)]=0;
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
					p[IX(i)][IY(j)][IZ(k)] = (div[IX(i)][IY(j)][IZ(k)] + p[IX(i-1)][IY(j)][IZ(k)] + p[IX(i+1)][IY(j)][IZ(k)] +
								p[IX(i)][IY(j-1)][IZ(k)] + p[IX(i)][IY(j+1)][IZ(k)] +
								p[IX(i)][IY(j)][IZ(k-1)] + p[IX(i)][IY(j)][IZ(k+1)])/6;
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
				u[IX(i)][IY(j)][IZ(k)] -= 0.5*(p[IX(i+1)][IY(j)][IZ(k)] - p[IX(i-1)][IY(j)][IZ(k)])/h;
				v[IX(i)][IY(j)][IZ(k)] -= 0.5*(p[IX(i)][IY(j+1)][IZ(k)] - p[IX(i)][IY(j-1)][IZ(k)])/h;
				w[IX(i)][IY(j)][IZ(k)] -= 0.5*(p[IX(i)][IY(j)][IZ(k+1)] - p[IX(i)][IY(j)][IZ(k-1)])/h;
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
	SWAP (x0, x); diffuse (N,0,x,x0,diff,dt);
	SWAP (x0, x); advect (N,0,x,x0,u,v,w,dt);
}

void vel_step (int N, float ***u, float ***v, float ***w, float ***u0, float ***v0, float ***w0, float visc, float dt)
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
	const int size = (N+2)*(N+2)*(N+2);
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

	zeroArray(N,dens);
	zeroArray(N,dens_prev);
	zeroArray(N,s);
	zeroArray(N,u);
	zeroArray(N,v);
	zeroArray(N,w);
	zeroArray(N,u_prev);
	zeroArray(N,v_prev);
	zeroArray(N,w_prev);

	dens_prev[IX(2)][IY(2)][IZ(2)] = 10.0f;

	for(int i = 0; i < (N+2); i++) { for(int j = 0; j < (N+2); j++) { for(int k = 0; k < (N+2); k++)
	{
		u[IX(i)][IY(j)][IZ(k)] = 10.0f;
		v[IX(i)][IY(j)][IZ(k)] = 10.0f;
		w[IX(i)][IY(j)][IZ(k)] = 10.0f;
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

	return 0;
}
