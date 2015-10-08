#define IX(i,j,k) ((i)+(N+2)*(j)+(N+2)*(N+2)*(k))
#define SWAP(x0, x) {float *tmp=x0;x0=x;x=tmp;}

//haxx så det funkar för tillfället
void *__gxx_personality_v0;

const int N=10;
const int size=(N+2)*(N+2)*(N+2);

static float u[size], v[size], w[size], u_prev[size], v_prev[size], w_prev[size];
static float dens[size], dens_prev[size];


void set_bnd( int N, int b, float* x)
{
	int i,j;
// needs loops over z
	for(i =1; i<=N; i++)
	{
		for(j=1;j<=N;j++)
		{
			//b = 1 means forces in x-axis
			x[IX(0,i,j)] = 	(b == 1? -x[IX(1,i,j)] : x[IX(1,i,j)]);
			x[IX(N+1,i,j)] = 	(b == 1? -x[IX(N,i,j)] : x[IX(N,i,j)]);
			//b == 2 means forces in y-axis
			x[IX(i,0,j)] = 	(b == 2? -x[IX(i,1,j)] : x[IX(i,1,j)]);
			x[IX(i,N+1,j)] =	(b == 2? -x[IX(i,N,j)] : x[IX(i,N,j)]);
			//b == 3 means forces in z-axis
			x[IX(i,j,0)] = 	(b == 3? -x[IX(i,j,1)] : x[IX(i,j,1)]);
			x[IX(i,j,N+1)] =	(b == 3? -x[IX(i,j,N)] : x[IX(i,j,N)]);
		}
	}

	x[IX(0, 0,0)] = 0.5*(x[IX(1,0,0)] + x[IX(0,1,0)]);
	x[IX(0, N+1,0)] = 0.5*(x[IX(1,N+1,0)] + x[IX(0,N+1,0)]);
	x[IX(N+1, 0,0)] = 0.5*(x[IX(N,0,0)] + x[IX(N+1,1,0)]);
	x[IX(N+1, N+1,0)] = 0.5*(x[IX(N,N+1,0)] + x[IX(N+1,N,0)]);
// 4 more
	x[IX(0, 0,N+1)] = 0.5*(x[IX(1,0,N+1)] + x[IX(0,1,N+1)]);
	x[IX(0, N+1,N+1)] = 0.5*(x[IX(1,N+1,N+1)] + x[IX(0,N+1,N+1)]);
	x[IX(N+1, 0,N+1)] = 0.5*(x[IX(N,0,N+1)] + x[IX(N+1,1,N+1)]);
	x[IX(N+1, N+1,N+1)] = 0.5*(x[IX(N,N+1,N+1)] + x[IX(N+1,N,N+1)]);
}





void add_source (int N, float *x, float *s, float dt)
{
	int i, size=(N+2)*(N+2)*(N+2);
	for (i=0;i<size;i++)
	{
		x[i] += dt*s[i];
	}
}

void diffuse ( int N, int b, float *x, float *x0, float diff, float dt)
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
				x[IX(i,j,k)] = (x0[IX(i,j,k)] + a*(
				x[IX(i-1,j,k)] + x[IX(i+1,j,k)] +
				x[IX(i,j-1,k)] + x[IX(i,j+1,k)] + 
				x[IX(i,j,k-1)] + x[IX(i,j,k+1)])    )/(1+6*a); //add two more also divide by 1+6*a
				}
			}
		}
		set_bnd (N,b,x);
	}

}

void advect (int N, int b, float *d, float *d0, float *u, float *v, float *w, float dt) // add float w
{
	int i,j,k,i0,j0,k0,i1,j1,k1;
	float x,y,z,s0,t0,q0,s1,t1,q1,dt0;

	// one loop in z needed
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

void project (int N, float *u, float *v, float *p, float *div) //add float *w
{
	int i,j,k;
	float h;

	h = 1.0/N;
	for (i=1;i<=N;i++)
	{
		for (j=1;j<=N;j++)
		{
			div[IX(i,j,0)] = -0.5*h*(u[IX(i+1,j,0)]-u[IX(i-1,j,0)]+
					v[IX(i,j+1,0)]-v[IX(i,j-1,0)]); //add forces in w
			p[IX(i,j,0)]=0;
		}
	}
	set_bnd (N,0,div); set_bnd (N,0,p);

	for (k=0;k<20;k++)
	{
		for (i=1;i<=N;i++)
		{
			for (j=1;j<=N;j++)
			{
				p[IX(i,j,0)] = (div[IX(i,j,0)] + p[IX(i-1,j,0)] + p[IX(i+1,j,0)] +
						p[IX(i,j-1,0)] + p[IX(i,j+1,0)])/4; //add for z, divide by 6
			}
		}
		 set_bnd(N, 0,p);	
	}

	for( i=1; i<=N; i++)
	{
		for (j=1; j<= N; j++)
		{
			u[IX(i,j,0)] -= 0.5*(p[IX(i+1,j,0)] - p[IX(i-1,j,0)])/h;
			v[IX(i,j,0)] -= 0.5*(p[IX(i,j+1,0)] - p[IX(i,j-1,0)])/h;
			// add w
		}
	}
	set_bnd(N,1,u);
	set_bnd(N,2,v);
// set bound for w
}	

void dens_step (int N, float *x, float *x0, float *u, float *v, float *w, float diff, float dt) //add float *w
{
	//NOT FIXED WITH W
	add_source (N,x,x0,dt);
	SWAP (x0, x); diffuse (N,0,x,x0,diff,dt);
	SWAP (x0, x); advect (N,0,x,x0,u,v,w,dt); //add w
}

void vel_step (int N, float *u, float *v, float *w, float *u0, float *v0, float *w0, float visc, float dt) //add w and w0
{

	//NOT FIXED WITH W
	add_source (N,u,u0,dt);
	add_source (N,v,v0,dt);
	//add w and w0
	SWAP (u0,u); diffuse (N,1,u,u0,visc,dt);
	SWAP (v0,v); diffuse (N,2,v,v0,visc,dt);
	//SWAP for w0 and w and diffuce
	project (N,u,v,u0,v0); //still swaped
	// include w and w0
	SWAP (u0,u);
	SWAP(v0,v);
	//SWAP for w0 and w
	advect (N,1,u,u0,u0,v0, w0,dt); // add w0
	advect (N,2,v,v0,u0,v0, w0,dt); // add w0
	// advect for w and w0
	project (N,u,v,u0,v0); //add w and w0
}


int main()
{

return 0;
}
