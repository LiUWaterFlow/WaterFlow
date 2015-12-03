#include "iostream"
#include <iomanip> 
#include <math.h>


const int N=21;
const int size=(N+2)*(N+2);


void set_bnd(int N, int b, float** x) 
{
	int i;

	for(i =0; i<=(N+1); i++)
	{
	//goes on the rand!
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
	//x[N+1][N+1] = 	0.5f*(x[N][N+1] + x[N+1][N]);

}

void abplusminus(float** aplus,float** aminus,float** bplus,float** bminus,float** H, float** U, float** V,float g,int N)
{
	float temp;
	for (int i = 0; i <= N; i++){
		for (int j = 0; j <= (N-1); j++){
			temp=(U[i+1][j+1]+sqrt(g*H[i+1][j+1])+(U[i][j+1]+sqrt(g*H[i][j+1])))/2;
			if (0 < temp)
			{
				aplus[i][j] = temp;
			}
			else
				aplus[i][j] = 0;

			temp=(U[i+1][j+1]-sqrt(g*H[i+1][j+1])+(U[i][j+1]-sqrt(g*H[i][j+1])))/2;
			if (0 > temp)
			{			
				aminus[i][j] = temp;
			}			
			else
				aminus[i][j] = 0;
		}
	}

	for (int i = 0; i <= (N-1); i++){
		for (int j = 0; j <= N; j++){
			temp=(V[i+1][j+1]+sqrt(g*H[i+1][j+1])+(V[i+1][j]+sqrt(g*H[i+1][j])))/2;
			if (0 < temp)
			{
				bplus[i][j] = temp;
			}			
			else
				bplus[i][j] = 0;

			temp=(V[i+1][j+1]-sqrt(g*H[i+1][j+1])+(V[i+1][j]-sqrt(g*H[i+1][j])))/2;
			if (0 > temp)
			{
				bminus[i][j] = temp;
			}
			else
				bminus[i][j] = 0;
		}
	}
}

void newH(float** B,float** w,float** H)
{
	for (int i = 0; i <= N+1; i++){
		for (int j = 0; j <= (N+1); j++){
			H[i][j]=w[i][j]-B[i][j];
		}
	}
}	

void Bhalf(float** BhalfX,float** BhalfY,float** BX,float** BY,float** B,int N)
{
	for (int i = 0; i <= N; i++){
		for (int j = 0; j <= (N-1); j++){
			BhalfX[i][j]=(B[i+1][j+1]+B[i][j+1])/2;
		}
	}

	for (int i = 0; i <= (N-1); i++){
		for (int j = 0; j <= N; j++){
			BhalfY[i][j]=(B[i+1][j+1]+B[i+1][j])/2;
		}
	}
	
	for (int i = 1; i <= N; i++){
		for (int j = 1; j <= N; j++){
			BX[i][j] = BhalfX[i-1][j-1] - BhalfX[i][j-1];
			BY[i][j] = BhalfY[i-1][j-1] - BhalfY[i-1][j];
			}
	}
}

void firsthalfstep(float** H,float** U,float** V,float** Hx,float** Ux,float** Vx,float** Hy,float** Uy,float** Vy,float** BhalfX,float** BhalfY,float** aplus,float** aminus,float** bplus,float** bminus,float** w,float dt,float dx,float dy,float g,int N)
{
	float c,u;

	for (int i = 0; i <= N; i++){
		for (int j = 0; j <= (N-1); j++){

			Hx[i][j] = (aplus[i][j]*H[i+1][j+1]*U[i+1][j+1] - aminus[i][j]*H[i][j+1]*U[i][j+1])/(aplus[i][j]-aminus[i][j]); //Hx is used for w
			Ux[i][j] = (aplus[i][j]*(H[i+1][j+1]*U[i+1][j+1]*H[i+1][j+1]*U[i+1][j+1]/(w[i+1][j+1]-BhalfX[i][j]) + g*((w[i+1][j+1]-BhalfX[i][j])*(w[i+1][j+1]-BhalfX[i][j]))/2) - aminus[i][j]*(H[i][j+1]*U[i][j+1]*H[i][j+1]*U[i][j+1]/(w[i][j+1]-BhalfX[i][j]) + g*((w[i][j+1]-BhalfX[i][j])*(w[i][j+1]-BhalfX[i][j]))/2))/(aplus[i][j]-aminus[i][j]);
			Vx[i][j] = (aplus[i][j]*(H[i+1][j+1]*U[i+1][j+1]*H[i+1][j+1]*V[i+1][j+1])/(w[i+1][j+1]-BhalfX[i][j]) - aminus[i][j]*(H[i][j+1]*U[i][j+1]*H[i][j+1]*V[i][j+1])/(w[i][j+1]-BhalfX[i][j]))/(aplus[i][j]-aminus[i][j]);
		}
	}

	for (int i = 0; i <= (N-1); i++){
		for (int j = 0; j <= N; j++){

			Hy[i][j] = (bplus[i][j]*H[i+1][j+1]*V[i+1][j+1] - bminus[i][j]*H[i+1][j]*V[i+1][j])/(bplus[i][j]-bminus[i][j]); //Hy is used for w
			Uy[i][j] = (bplus[i][j]*(H[i+1][j+1]*U[i+1][j+1]*H[i+1][j+1]*V[i+1][j+1])/(w[i+1][j+1]-BhalfY[i][j]) - bminus[i][j]*(H[i+1][1]*U[i+1][1]*H[i+1][1]*V[i+1][1])/(w[i+1][j]-BhalfY[i][j]))/(bplus[i][j]-bminus[i][j]);
			Vy[i][j] = (bplus[i][j]*(H[i+1][j+1]*V[i+1][j+1]*H[i+1][j+1]*V[i+1][j+1]/(w[i+1][j+1]-BhalfY[i][j]) + g*((w[i+1][j+1]-BhalfY[i][j])*(w[i+1][j+1]-BhalfY[i][j]))/2) - bminus[i][j]*(H[i+1][j]*V[i+1][j]*H[i+1][j]*V[i+1][j]/(w[i+1][j]-BhalfY[i][j]) + g*((w[i+1][j]-BhalfY[i][j])*(w[i+1][j]-BhalfY[i][j]))/2))/(bplus[i][j]-bminus[i][j]);
		}
	}
}

void secondhalfstep(float** H,float** U,float** V,float** Hx,float** Ux,float** Vx,float** Hy,float** Uy,float** Vy,float** B,float** BX,float** BY,float** w,float dt,float dx,float dy,float g,int N)
{

	for (int i = 1; i <= N; i++){
		for (int j = 1; j <= N; j++){

			w[i][j] = w[i][j] - (dt/dx)*(Hx[i][j-1]-Hx[i-1][j-1]) - (dt/dy)*(Hy[i-1][j]-Hy[i-1][j-1]) + dt*0;
			U[i][j] = U[i][j] - (dt/dx)*(Ux[i][j-1]-Ux[i-1][j-1]) - (dt/dy)*(Uy[i-1][j]-Uy[i-1][j-1]) + dt*((B[i][j]-w[i][j])*BX[i][j]);//fixa w B Bgrad
			V[i][j] = V[i][j] - (dt/dx)*(Vx[i][j-1]-Vx[i-1][j-1]) - (dt/dy)*(Vy[i-1][j]-Vy[i-1][j-1]) + dt*((B[i][j]-w[i][j])*BY[i][j]);
		}
	}
	
	for (int i = 0; i <= (N+1); i++){
		for (int j = 0; j <= (N+1); j++){
			if (w[i][j] < B[i][j])
			{
			w[i][j] = B[i][j];//+0.001f;
			}
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
			std::cout << std::fixed << std::setprecision(3) << v[x][y] << " ";
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

void BArray(const int N, float** x)
{
	for(int i = 0; i <= (N+1); i++)
	{
		for(int j = 0; j <= (N+1); j++)
		{

			x[i][j] = 0.0f;
			
		}
	}
	
	x[3][1] = 0.1f;

	x[9][10] = 0.2f;
	x[10][9] = 0.2f;
	x[10][10] = 0.3f;
	x[10][11] = 0.2f;
	x[11][10] = 0.2f;

	x[15][16] = 0.2f;
	x[16][15] = 0.2f;
	x[16][16] = 0.3f;
	x[16][17] = 0.2f;
	x[17][16] = 0.2f;

}

void wArray(const int N, float** x)
{
	for(int i = 0; i <= (N+1); i++)
	{
		for(int j = 0; j <= (N+1); j++)
		{

			x[i][j] = 0.2f;
			
		}
	}	

	for(int i = 9; i <= 17; i++)
	{
		for(int j = 9; j <= 17; j++)
		{

			x[i][j] = 0.3f;
			
		}
	}


	
/*
	x[12][13] = 0.6f;
	x[13][12] = 0.6f;
	x[13][13] = 0.7f;
	x[13][14] = 0.6f;
	x[14][13] = 0.6f;
*/	


	//droppe

	//x[3][3]=5.5f;
/*
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
/*
	//dambreak
	for(int i = 0; i <= (N-14); i++)
	{
		for(int j = 0; j <= (N+1); j++)
		{
			x[i][j] = 1.5f; //high value doesnt work well e.g. dam break
			
		}
	}
*/
}


int main()
{

	int N = 21;
	//int N = 20;
	float g = 9.8f;
	float dt = 0.01f;
	float dx = 1.0f;
	float dy = 1.0f;

	float** w = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		w[i] = new float[N+2];
	}

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

	float** aplus = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		aplus[i] = new float[N+2];
	}

	float** aminus = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		aminus[i] = new float[N+2];
	}

	float** bplus = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		bplus[i] = new float[N+2];
	}

	float** bminus = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		bminus[i] = new float[N+2];
	}

	float** B = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		B[i] = new float[N+2];
	}

	float** BX = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		BX[i] = new float[N+2];
	}

	float** BY = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		BY[i] = new float[N+2];
	}

	float** BhalfX = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		BhalfX[i] = new float[N+2];
	}

	float** BhalfY = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		BhalfY[i] = new float[N+2];
	}

	wArray(N,w);

	zeroArray(N,H);
	zeroArray(N,U);
	zeroArray(N,V);
	zeroArray(N,Hx);
	zeroArray(N,Ux);
	zeroArray(N,Vx);
	zeroArray(N,Hy);
	zeroArray(N,Uy);
	zeroArray(N,Vy);

	zeroArray(N,aplus);
	zeroArray(N,aminus);
	zeroArray(N,bplus);
	zeroArray(N,bminus);
	//zeroArray(N,B);
	zeroArray(N,BX);
	zeroArray(N,BY);
	zeroArray(N,BhalfX);
	zeroArray(N,BhalfY);




	BArray(N,B);

	Bhalf(BhalfX,BhalfY,BX,BY,B,N);

	for(int i=0; i<= 600; i++)
	{
		
	set_bnd(N,0,H);
	set_bnd(N,1,U);
	set_bnd(N,2,V);
	
	newH(B,w,H);

	abplusminus(aplus,aminus,bplus,bminus,H,U,V,g,N);

	firsthalfstep(H,U,V,Hx,Ux,Vx,Hy,Uy,Vy,BhalfX,BhalfY,aplus,aminus,bplus,bminus,w,dt,dx,dy,g,N);

	secondhalfstep(H,U,V,Hx,Ux,Vx,Hy,Uy,Vy,B,BX,BY,w,dt,dx,dy,g,N);

/*	
	if (i==10)
	{
		H[10][10]=2.0f;	
	}

	std::cout << "Printing Hx:" << std::endl;
	print(N,Hx);
	std::cout << "Printing Hy:" << std::endl;
	print(N,Hy);
	std::cout << "Printing H:" << std::endl;
	print(N,H);
	std::cout << "Printing U:" << std::endl;
	print(N,U);
	std::cout << "Printing Ux:" << std::endl;
	print(N,Ux);	
	std::cout << "Printing V:" << std::endl;
	print(N,V);	
	std::cout << "Printing Vy:" << std::endl;
	print(N,Vy);	
*/
	std::cout << "Printing w:" << std::endl;
	print(N,w);
	}



	//std::cout << "Printing H:" << std::endl;
	//print(N,H);


	//std::cout << "Printing U:" << std::endl;
	//print(N,U);
	//std::cout << "Printing V:" << std::endl;
	//print(N,V);

	//std::cout << "Printing Hx:" << std::endl;
	//print(N,Hx);
	//std::cout << "Printing Hy:" << std::endl;
	//print(N,Hy);
	//std::cout << "Printing H:" << std::endl;
	//print(N,H);

	std::cout << "Printing B:" << std::endl;
	print(N,B);
	//std::cout << "Printing w:" << std::endl;
	//print(N,w);


	for (int i = 0; i < (N+2); i++){
		delete [] w[i];
	}
	delete [] w;
	
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

	for (int i = 0; i < (N+2); i++){
		delete [] aplus[i];
	}
	delete [] aplus;

	for (int i = 0; i < (N+2); i++){
		delete [] aminus[i];
	}
	delete [] aminus;

	for (int i = 0; i < (N+2); i++){
		delete [] bplus[i];
	}
	delete [] bplus;

	for (int i = 0; i < (N+2); i++){
		delete [] bminus[i];
	}
	delete [] bminus;

	for (int i = 0; i < (N+2); i++){
		delete [] B[i];
	}
	delete [] B;

	for (int i = 0; i < (N+2); i++){
		delete [] BX[i];
	}
	delete [] BX;

	for (int i = 0; i < (N+2); i++){
		delete [] BY[i];
	}
	delete [] BY;

	for (int i = 0; i < (N+2); i++){
		delete [] BhalfX[i];
	}
	delete [] BhalfX;

	for (int i = 0; i < (N+2); i++){
		delete [] BhalfY[i];
	}
	delete [] BhalfY;


	return 0;
}

