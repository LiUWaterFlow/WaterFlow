#include "iostream"
//#include "fluidsolver.h"

//haxx så det funkar för tillfället
//void *__gxx_personality_v0;

//const int N=64;
const int N=20;
const int size=(N+2)*(N+2);


void set_bnd(int N, int b, float** x) 
{
	int i;

	for(i =1; i<=N; i++)
	{
		//b = 1 means forces in x-axis
		x[0][i] = 		(b == 1? -x[1][i] : x[1][i]);
		x[N+1][i] = 		(b == 1? -x[N][i] : x[N][i]);
		//b == 2 means forces in y-axis
		x[i][0] = 		(b == 2? -x[i][1] : x[i][1]);
		x[i][N+1] =		(b == 2? -x[i][N] : x[i][N]);
	}

	//x[0][0] = 		0.5f*(x[1][0] + x[0][1]);
	//x[0][N+1] =	 	0.5f*(x[1][N+1] + x[0][N]);
	//x[N+1][0] = 		0.5f*(x[N+1][1] + x[N][0]);
	//x[N+1][N+1] = 		0.5f*(x[N][N+1] + x[N+1][N]);

}

void firsthalfstep(float** H,float** U,float** V,float** Hx,float** Ux,float** Vx,float** Hy,float** Uy,float** Vy,float dt,float dx,float dy,float g,int N)
{
	for (int i = 0; i <= N; i++){
		for (int j = 0; j <= (N-1); j++){
			Hx[i][j] = (H[i+1][j+1] + H[i][j+1])/2 - dt/(2*dx)*(U[i+1][j+1] - U[i][j+1]);
			Ux[i][j] = (U[i+1][j+1] + U[i][j+1])/2 - dt/(2*dx)*((U[i+1][j+1]*U[i+1][j+1])/H[i+1][j+1] + g/2*(H[i+1][j+1]*H[i+1][j+1])) - (U[i][j+1]*U[i][j+1])/H[i][j+1] + g/2*(H[i][j+1]*H[i][j+1]);
			Vx[i][j] = (V[i+1][j+1] + V[i][j+1])/2 - dt/(2*dx)*((U[i+1][j+1]*V[i+1][j+1]/H[i+1][j+1]) - (U[i][j+1]*V[i][j+1]/H[i][j+1]));
		}
	}

	for (int i = 0; i <= (N-1); i++){
		for (int j = 0; j <= N; j++){
			Hy[i][j] = (H[i+1][j+1] + H[i+1][j])/2 - dt/(2*dy)*(V[i+1][j+1] - V[i+1][j]);
			Uy[i][j] = (U[i+1][j+1] + U[i+1][j])/2 - dt/(2*dy)*((V[i+1][j+1]*U[i+1][j+1]/H[i+1][j+1]) - (V[i+1][j]*U[i+1][j]/H[i+1][j]));
			Vy[i][j] = (V[i+1][j+1] + V[i+1][j])/2 - dt/(2*dy)*((V[i+1][j+1]*V[i+1][j+1]/H[i+1][j+1] + g/2*H[i+1][j+1]*H[i+1][j+1]) - (V[i+1][j]*V[i+1][j]/H[i+1][j] + g/2*H[i+1][j]*H[i+1][j]));
		}
	}
}

void secondhalfstep(float** H,float** U,float** V,float** Hx,float** Ux,float** Vx,float** Hy,float** Uy,float** Vy,float dt,float dx,float dy,float g,int N)
{
	for (int i = 1; i <= N; i++){
		for (int j = 1; j <= N; j++){
			H[i][j] = H[i][j] - (dt/dx)*(Ux[i][j-1]-Ux[i-1][j-1]) - (dt/dy)*(Vy[i-1][j]-Vy[i-1][j-1]);
			U[i][j] = U[i][j] - (dt/dx)*((Ux[i][j-1]*Ux[i][j-1]/Hx[i][j-1] + g/2*Hx[i][j-1]*Hx[i][j-1]) - (Ux[i-1][j-1]*Ux[i-1][j-1]/Hx[i-1][j-1] + g/2*Hx[i-1][j-1]*Hx[i-1][j-1])) - (dt/dy)*((Vy[i-1][j]*Uy[i-1][j]/Hy[i-1][j]) - (Vy[i-1][j-1]*Uy[i-1][j-1]/Hy[i-1][j-1]));
			V[i][j] = V[i][j] - (dt/dx)*((Ux[i][j-1]*Vx[i][j-1]/Hx[i][j-1]) - (Ux[i-1][j-1]*Vx[i-1][j-1]/Hx[i-1][j-1])) - (dt/dy)*((Vy[i-1][j]*Vy[i-1][j]/Hy[i-1][j] + g/2*Hy[i-1][j]*Hy[i-1][j]) - (Vy[i-1][j-1]*Vy[i-1][j-1]/Hy[i-1][j-1] + g/2*Hy[i-1][j-1]*Hy[i-1][j-1]));	
		}
	}
}



float sumArray(int N, float** v)
{
float sum = 0;

	for(int y = 1; y <= N; y++)
	{
		for(int x = 1; x <= N; x++)
		{
			sum += v[x][y];
		}
	}

	return sum;
}

void print(int N, float** v)
{

	for(int y = 1; y <= N; y++)
	{
		for(int x = 1; x <= N; x++)
		{
			std::cout << v[x][y] << " ";
		}
		std::cout << "\n";
	}

	std::cout << "Sum is: " << sumArray(N,v) << "\n";
	std::cout << std::endl;
}

void zeroArray(const int N, float** x)
{
	for(int i = 0; i <= (N+1); i++)
	{
		for(int j = 0; j <= (N+1); j++)
		{

			x[i][j] = 0.0f;
			
		}
	}
}

void oneArray(const int N, float** x)
{
	for(int i = 0; i <= (N+1); i++)
	{
		for(int j = 0; j <= (N+1); j++)
		{

			x[i][j] = 1.0f;
			
		}
	}

	
/*

	//droppe

	x[8][5]=1.1f;
	x[9][4]=1.1f;
	x[9][5]=1.3f;
	x[9][6]=1.1f;
	x[10][3]=1.1f;
	x[10][4]=1.3f;
	x[10][5]=1.5f;
	x[10][6]=1.3f;
	x[10][7]=1.1f;
	x[11][4]=1.1f;
	x[11][5]=1.3f;
	x[11][6]=1.1f;
	x[12][5]=1.1f;
*/
	//dambreak
	for(int i = 0; i <= (N-14); i++)
	{
		for(int j = 0; j <= (N+1); j++)
		{

			x[i][j] = 5.0f; //high value doesnt work well e.g. dam break
			
		}
	}

}

int main()
{

	//int N = 64;
	int N = 20;
	float g = 9.8f;
	float dt = 0.01f;
	float dx = 1.0f;
	float dy = 1.0f;

	float** H = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		H[i] = new float[N+2];
	}

	float** U = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		U[i] = new float[N+2];
	}

	float** V = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		V[i] = new float[N+2];
	}

	float** Hx = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		Hx[i] = new float[N+2];
	}

	float** Ux = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		Ux[i] = new float[N+2];
	}

	float** Vx = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		Vx[i] = new float[N+2];
	}

	float** Hy = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		Hy[i] = new float[N+2];
	}

	float** Uy = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		Uy[i] = new float[N+2];
	}

	float** Vy = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		Vy[i] = new float[N+2];
	}


	oneArray(N,H);
	zeroArray(N,U);
	zeroArray(N,V);
	zeroArray(N,Hx);
	zeroArray(N,Ux);
	zeroArray(N,Vx);
	zeroArray(N,Hy);
	zeroArray(N,Uy);
	zeroArray(N,Vy);

	print(N,H);

	for(int i=0; i<= 20; i++)
	{
		
	set_bnd(N,0,H);
	set_bnd(N,1,U);
	set_bnd(N,2,V);

	firsthalfstep(H,U,V,Hx,Ux,Vx,Hy,Uy,Vy,dt,dx,dy,g,N);
	secondhalfstep(H,U,V,Hx,Ux,Vx,Hy,Uy,Vy,dt,dx,dy,g,N);

	print(N,H);

	}
	
	for (int i = 0; i < (N+2); i++){
		delete [] H[i];
	}
	delete [] H;

	for (int i = 0; i < (N+2); i++){
		delete [] U[i];
	}
	delete [] U;

	for (int i = 0; i < (N+2); i++){
		delete [] V[i];
	}
	delete [] V;

	for (int i = 0; i < (N+2); i++){
		delete [] Hx[i];
	}
	delete [] Hx;

	for (int i = 0; i < (N+2); i++){
		delete [] Ux[i];
	}
	delete [] Ux;

	for (int i = 0; i < (N+2); i++){
		delete [] Vx[i];
	}
	delete [] Vx;

	for (int i = 0; i < (N+2); i++){
		delete [] Hy[i];
	}
	delete [] Hy;

	for (int i = 0; i < (N+2); i++){
		delete [] Uy[i];
	}
	delete [] Uy;

	for (int i = 0; i < (N+2); i++){
		delete [] Vy[i];
	}
	delete [] Vy;


	return 0;
}

