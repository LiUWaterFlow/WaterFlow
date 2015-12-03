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

void increaseWater(float** w,int N)
{
	for (int i = 0; i <= (N+1); i++){
		for (int j = 0; j <= (N+1); j++){
			w[i][j]=w[i][j]+1.0f;
		}
	}
}

void decreaseWater(float** w,int N)
{
	for (int i = 0; i <= (N+1); i++){
		for (int j = 0; j <= (N+1); j++){
			w[i][j]=w[i][j]-1.0f;
		}
	}
}

void Bhalf(float** BhalfX,float** BhalfY,float** BX,float** BY,float** BhalfC,float** B,int N)
{
	for (int i = 0; i <= N; i++){
		for (int j = 0; j <= N; j++){
			BhalfX[i][j]=(BhalfC[i][j+1]+BhalfC[i][j])/2; //since BhalfC is between both in i-axis and j-axis
		}
	}

	for (int i = 0; i <= N; i++){
		for (int j = 0; j <= N; j++){
			BhalfY[i][j]=(BhalfC[i+1][j]+BhalfC[i][j])/2;
		}
	}
	
	for (int i = 1; i <= N; i++){
		for (int j = 1; j <= N; j++){
			BX[i][j] = BhalfX[i-1][j] - BhalfX[i][j];
			BY[i][j] = BhalfY[i][j-1] - BhalfY[i][j];
			}
	}

	for (int i = 1; i <= N; i++){
		for (int j = 1; j <= N; j++){
			B[i][j] = (BhalfX[i-1][j] + BhalfX[i][j] + BhalfY[i][j-1] + BhalfY[i][j])/4; //B is not defined in N=0 or N=N+1
			}
	}
}

void EastWestStep(float** grid, float** Estep,float** Wstep,float** heightE,float** heightW,float** BhalfX,int N)
{
	for (int i = 0; i <= (N+1); i++){
		for (int j = 0; j <= (N+1); j++){
			Estep[i][j]=grid[i][j];
		}
	}

	for (int i = 0; i <= (N+1); i++){
		for (int j = 0; j <= (N+1); j++){
			Wstep[i][j]=grid[i][j];
		}
	}

	for (int i = 0; i <= N; i++){
		for (int j = 1; j <= N; j++){ 
			if (Estep[i][j] < BhalfX[i][j])
			{
			float TempStep = Estep[i][j];
			Estep[i][j] = BhalfX[i][j];
			Wstep[i][j] = TempStep + Wstep[i][j] - BhalfX[i][j];
			}
		}
	}

	for (int i = 1; i <= (N+1); i++){
		for (int j = 1; j <= N; j++){
			if (Wstep[i][j] < BhalfX[i-1][j]) //obs j-1/2
			{
			float TempStep = Wstep[i][j];
			Wstep[i][j] = BhalfX[i-1][j];
			Estep[i][j] = TempStep + Estep[i][j] - BhalfX[i-1][j];
			}
		}
	}

	for (int i = 0; i <= N; i++){
		for (int j = 1; j <= N; j++){
			heightE[i][j] = Estep[i][j] - BhalfX[i][j];
		}
	}

	for (int i = 1; i <= (N+1); i++){
		for (int j = 1; j <= N; j++){
			heightW[i][j] = Wstep[i][j] - BhalfX[i-1][j];
		}
	}
}

void NorthSouthStep(float** grid, float** Nstep,float** Sstep,float** heightN,float** heightS,float** BhalfY,int N) //obs north is up, thus k-1/2, reversed from report
{
	for (int i = 0; i <= (N+1); i++){
		for (int j = 0; j <= (N+1); j++){
			Sstep[i][j]=grid[i][j];
		}
	}

	for (int i = 0; i <= (N+1); i++){
		for (int j = 0; j <= (N+1); j++){
			Nstep[i][j]=grid[i][j];
		}
	}

	for (int i = 1; i <= N; i++){
		for (int j = 0; j <= N; j++){
			if (Sstep[i][j] < BhalfY[i][j])
			{
			float TempStep = Sstep[i][j];
			Sstep[i][j] = BhalfY[i][j];
			Nstep[i][j] = TempStep + Nstep[i][j] - BhalfY[i][j];
			}
		}
	}

	for (int i = 1; i <= N; i++){
		for (int j = 1; j <= (N+1); j++){
			if (Nstep[i][j] < BhalfY[i][j-1]) //obs k-1/2
			{
			float TempStep = Nstep[i][j];
			Nstep[i][j] = BhalfY[i][j-1];
			Sstep[i][j] = TempStep + Sstep[i][j] - BhalfY[i][j-1];
			}
		}
	}

	for (int i = 1; i <= N; i++){
		for (int j = 0; j <= N; j++){
			heightS[i][j] = Sstep[i][j] - BhalfY[i][j];
		}
	}

	for (int i = 1; i <= N; i++){
		for (int j = 1; j <= (N+1); j++){
			heightN[i][j] = Nstep[i][j] - BhalfY[i][j-1];	
		}
	}
}

void calculateVelocityandMomentum(float** momentumC, float** momentumDirection,float** heightDirection,float** velocityDirection, float epsilon) //get HU (for east and west) and HV (for south and north)
{
	float heightFour;
	float velocity; //u or v

	for (int i = 0; i <= (N+1); i++){
		for (int j = 0; j <= (N+1); j++){	
			heightFour=pow(heightDirection[i][j], 4.0);
			if (heightFour > epsilon)
			{
				velocityDirection[i][j] = sqrt(2)*heightDirection[i][j]*momentumC[i][j]/sqrt(heightFour + heightFour); //unsure about momentumC, should it be halfstep or fullstep
			}
			else
			{
				velocityDirection[i][j] = sqrt(2)*heightDirection[i][j]*momentumC[i][j]/sqrt(heightFour + epsilon);
			}
			momentumDirection[i][j] = velocityDirection[i][j] * heightDirection[i][j];
		}
	}
}

void speedOfPropagation(float** aplus,float** aminus,float** bplus,float** bminus,float** velocityE,float** velocityW,float** velocityS,float** velocityN,float** heightE,float** heightW,float** heightS,float** heightN,float g,int N)
{
	float temp1,temp2;
	for (int i = 0; i <= N; i++){
		for (int j = 0; j <= N; j++){

			temp1=velocityE[i][j]+sqrt(g*heightE[i][j]);
			temp2=velocityW[i+1][j]+sqrt(g*heightW[i+1][j]);
			if (temp1 < temp2)
			{
				temp1=temp2;
			}
			if (temp1 < 0)
			{
				temp1=0;		
			}
			aplus[i][j]=temp1;

			temp1=velocityE[i][j]-sqrt(g*heightE[i][j]);
			temp2=velocityW[i+1][j]-sqrt(g*heightW[i+1][j]);
			if (temp1 > temp2)
			{
				temp1=temp2;
			}
			if (temp1 > 0)
			{
				temp1=0;		
			}
			aminus[i][j]=temp1;

			temp1=velocityS[i][j]+sqrt(g*heightS[i][j]);
			temp2=velocityN[i][j+1]+sqrt(g*heightN[i][j+1]);
			if (temp1 < temp2)
			{
				temp1=temp2;
			}
			if (temp1 < 0)
			{
				temp1=0;		
			}
			bplus[i][j]=temp1;

			temp1=velocityS[i][j]-sqrt(g*heightS[i][j]);
			temp2=velocityN[i][j+1]-sqrt(g*heightN[i][j+1]);
			if (temp1 > temp2)
			{
				temp1=temp2;
			}
			if (temp1 > 0)
			{
				temp1=0;		
			}
			bminus[i][j]=temp1;
		}
	}
}

void firsthalfstep(float** HxOne,float** HxTwo,float** HxThr,float** HyOne,float** HyTwo,float** HyThr,float** aplus,float** aminus,float** bplus,float** bminus,float** huE,float** huW,float** huN,float** huS,float** hvE,float** hvW,float** hvN,float** hvS,float** wE,float** wW,float** wN,float** wS,float** BhalfX,float** BhalfY,float g,int N)
{

	float wfirstTemp;
	float wsecondTemp;

	for (int i = 0; i <= N; i++){
		for (int j = 0; j <= (N+1); j++){

			wfirstTemp=wE[i][j];
			wsecondTemp=wW[i+1][j];

			if (huE[i][j] <= 0)
			{
				wfirstTemp=wE[i][j]+0.1f; //avoid divide zero by zero
			}
			if (huW[i+1][j] <= 0)
			{
				wsecondTemp=wW[i+1][j]+0.1f; //avoid divide zero by zero
			}

			HxOne[i][j] = (aplus[i][j]*huE[i][j] - aminus[i][j]*huW[i+1][j])/(aplus[i][j]-aminus[i][j]) + aplus[i][j]*aminus[i][j]/(aplus[i][j]-aminus[i][j])*(wW[i+1][j]-wE[i][j]); //Hx is used for w
			
			HxTwo[i][j] = (aplus[i][j]*(huE[i][j]*huE[i][j]/(wfirstTemp-BhalfX[i][j]) + g*((wE[i][j]-BhalfX[i][j])*(wE[i][j]-BhalfX[i][j]))/2) - aminus[i][j]*(huW[i+1][j]*huW[i+1][j]/(wsecondTemp-BhalfX[i][j]) + g*((wW[i+1][j]-BhalfX[i][j])*(wW[i+1][j]-BhalfX[i][j]))/2))/(aplus[i][j]-aminus[i][j]) + aplus[i][j]*aminus[i][j]/(aplus[i][j]-aminus[i][j])*(huW[i+1][j]-huE[i][j]);
			
			HxThr[i][j] = (aplus[i][j]*huE[i][j]*hvE[i][j]/(wfirstTemp-BhalfX[i][j]) - aminus[i][j]*huW[i+1][j]*hvW[i+1][j]/(wsecondTemp-BhalfX[i][j]))/(aplus[i][j]-aminus[i][j]) + aplus[i][j]*aminus[i][j]/(aplus[i][j]-aminus[i][j])*(hvW[i+1][j]-hvE[i][j]);

		}
	}

	for (int i = 0; i <= (N+1); i++){
		for (int j = 0; j <= N; j++){

			wfirstTemp=wS[i][j];
			wsecondTemp=wN[i][j+1];

			if (huS[i][j] <= 0)
			{
				wfirstTemp=wS[i][j]+0.1f; //avoid divide zero by zero
			}
			if (huN[i][j+1] <= 0)
			{
				wsecondTemp=wN[i][j+1]+0.1f; //avoid divide zero by zero
			}

			HyOne[i][j] = (bplus[i][j]*hvS[i][j] - bminus[i][j]*hvN[i][j+1])/(bplus[i][j]-bminus[i][j]) + bplus[i][j]*bminus[i][j]/(bplus[i][j]-bminus[i][j])*(wN[i][j+1]-wS[i][j]); //Hx is used for w
			
			HyTwo[i][j] = (bplus[i][j]*huS[i][j]*hvS[i][j]/(wfirstTemp-BhalfY[i][j]) - bminus[i][j]*huN[i][j+1]*hvN[i][j+1]/(wsecondTemp-BhalfY[i][j]))/(bplus[i][j]-bminus[i][j]) + bplus[i][j]*bminus[i][j]/(bplus[i][j]-bminus[i][j])*(huN[i][j+1]-huS[i][j]);
			
			HyThr[i][j] = (bplus[i][j]*(hvS[i][j]*hvS[i][j]/(wfirstTemp-BhalfY[i][j]) + g*((wS[i][j]-BhalfY[i][j])*(wS[i][j]-BhalfY[i][j]))/2) - bminus[i][j]*(hvN[i][j+1]*hvN[i][j+1]/(wsecondTemp-BhalfY[i][j]) + g*((wN[i][j+1]-BhalfY[i][j])*(wN[i][j+1]-BhalfY[i][j]))/2))/(bplus[i][j]-bminus[i][j]) + bplus[i][j]*bminus[i][j]/(bplus[i][j]-bminus[i][j])*(hvN[i][j+1]-hvS[i][j]);			

		}
	}
}

void secondhalfstep(float** w,float** hu,float** hv,float** HxOne,float** HxTwo,float** HxThr,float** HyOne,float** HyTwo,float** HyThr,float** B,float** BX,float** BY,float dt,float dx,float dy,int N)
{

	for (int i = 1; i <= (N+1); i++){
		for (int j = 1; j <= (N+1); j++){

			w[i][j] = w[i][j] - (dt/dx)*(HxOne[i][j] - HxOne[i-1][j]) - (dt/dy)*(HyOne[i][j]-HyOne[i][j-1]) + dt*0;
			hu[i][j] = hu[i][j] - (dt/dx)*(HxTwo[i][j] - HxTwo[i-1][j]) - (dt/dy)*(HyTwo[i][j]-HyTwo[i][j-1]) + dt*((B[i][j]-w[i][j])*BX[i][j]);//fixa w B Bgrad
			hv[i][j] = hv[i][j] - (dt/dx)*(HxThr[i][j] - HxThr[i-1][j]) - (dt/dy)*(HyThr[i][j]-HyThr[i][j-1]) + dt*((B[i][j]-w[i][j])*BY[i][j]);
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

void fillwithWater(float** B,float** w,int N)
{
	for(int i = 0; i <= (N+1); i++)
	{
		for(int j = 0; j <= (N+1); j++)
		{
			w[i][j] = B[i][j];
		}
	}

	for(int i = 9; i <= 17; i++)
	{
		for(int j = 9; j <= 17; j++)
		{

			w[i][j] = 0.4f;
			
		}
	}
}
void wArray(const int N, float** x)
{
	for(int i = 0; i <= (N+1); i++)
	{
		for(int j = 0; j <= (N+1); j++)
		{

			x[i][j] = 0.01f;
			
		}
	}	

	for(int i = 9; i <= 17; i++)
	{
		for(int j = 9; j <= 17; j++)
		{

			x[i][j] = 0.4f;
			
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

float calculateEpsilon(float dx,float dy,float epsilon)
{
	float dxFour=dx*dx*dx*dx;
	float dyFour=dy*dy*dy*dy;

	//dxFour=pow(dx, 4.0);
	//dyFour=pow(dy, 4.0);

	//dxFour=dx*dx*dx*dx;
	//dyFour=dy*dy*dy*dy;

	if (dxFour>dyFour)
	{
		epsilon=dxFour;
	}

	epsilon=dyFour;
	return epsilon;
}

int main()
{

	int N = 21;
	//int N = 20;
	float g = 9.8f;
	float dt = 0.01f;
	float dx = 1.0f;
	float dy = 1.0f;
	float epsilon = 0.0f;

	float** w = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		w[i] = new float[N+2];
	}


	float** wE = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		wE[i] = new float[N+2];
	}

	float** wW = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		wW[i] = new float[N+2];
	}

	float** wN = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		wN[i] = new float[N+2];
	}

	float** wS = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		wS[i] = new float[N+2];
	}


	float** hu = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		hu[i] = new float[N+2];
	}

	float** hv = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		hv[i] = new float[N+2];
	}


	float** hE = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		hE[i] = new float[N+2];
	}

	float** hW = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		hW[i] = new float[N+2];
	}

	float** hN = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		hN[i] = new float[N+2];
	}

	float** hS = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		hS[i] = new float[N+2];
	}

	float** uE = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		uE[i] = new float[N+2];
	}

	float** uW = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		uW[i] = new float[N+2];
	}

	float** uN = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		uN[i] = new float[N+2];
	}

	float** uS = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		uS[i] = new float[N+2];
	}

	float** huE = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		huE[i] = new float[N+2];
	}

	float** huW = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		huW[i] = new float[N+2];
	}

	float** huN = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		huN[i] = new float[N+2];
	}

	float** huS = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		huS[i] = new float[N+2];
	}

	float** vE = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		vE[i] = new float[N+2];
	}

	float** vW = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		vW[i] = new float[N+2];
	}

	float** vN = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		vN[i] = new float[N+2];
	}

	float** vS = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		vS[i] = new float[N+2];
	}

	float** hvE = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		hvE[i] = new float[N+2];
	}

	float** hvW = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		hvW[i] = new float[N+2];
	}

	float** hvN = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		hvN[i] = new float[N+2];
	}

	float** hvS = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		hvS[i] = new float[N+2];
	}

	float** HxOne = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		HxOne[i] = new float[N+2];
	}

	float** HxTwo = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		HxTwo[i] = new float[N+2];
	}

	float** HxThr = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		HxThr[i] = new float[N+2];
	}

	float** HyOne = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		HyOne[i] = new float[N+2];
	}

	float** HyTwo = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		HyTwo[i] = new float[N+2];
	}

	float** HyThr = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		HyThr[i] = new float[N+2];
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

	float** BhalfC = new float*[N+2];
	for (int i = 0; i < N+2; i++){
		BhalfC[i] = new float[N+2];
	}

	wArray(N,w);
	zeroArray(N,wE);
	zeroArray(N,wW);
	zeroArray(N,wN);
	zeroArray(N,wS);

	zeroArray(N,hu);
	zeroArray(N,hv);

	zeroArray(N,hE);
	zeroArray(N,hW);
	zeroArray(N,hS);
	zeroArray(N,hN);

	zeroArray(N,uE);
	zeroArray(N,uW);
	zeroArray(N,uS);
	zeroArray(N,uN);

	zeroArray(N,vE);
	zeroArray(N,vW);
	zeroArray(N,vS);
	zeroArray(N,vN);

	zeroArray(N,huE);
	zeroArray(N,huW);
	zeroArray(N,huS);
	zeroArray(N,huN);

	zeroArray(N,hvE);
	zeroArray(N,hvW);
	zeroArray(N,hvS);
	zeroArray(N,hvN);

	//speedOfPropagation
	zeroArray(N,aplus);
	zeroArray(N,aminus);
	zeroArray(N,bplus);
	zeroArray(N,bminus);

	//Topography
	zeroArray(N,B);
	zeroArray(N,BX);
	zeroArray(N,BY);
	zeroArray(N,BhalfX);
	zeroArray(N,BhalfY);

	BArray(N,BhalfC);

	//Calculate Topography
	//Bhalf(BhalfX,BhalfY,BX,BY,B,N);

	Bhalf(BhalfX,BhalfY,BX,BY,BhalfC,B,N);

	epsilon=calculateEpsilon(dx,dy,epsilon);

	fillwithWater(B,w,N);

//Clear view for 50, if 490 you can see first error
	for(int i=0; i<= 490; i++)
	{
		
	set_bnd(N,0,w);
	set_bnd(N,1,hu);
	set_bnd(N,2,hv);
	
	increaseWater(w,N); //To avoid divide by zero

	EastWestStep(w,wE,wW,hE,hW,BhalfX,N); //calculate water surface (w) and water height from ground (h) in east and west step
	NorthSouthStep(w,wN,wS,hN,hS,BhalfY,N); //calculate water surface (w) and water height from ground (h) in north and south step

	//calculate new huE,huW,huN,huS,uE,uW,uN,uS (momentum and velocity)
	calculateVelocityandMomentum(hu,huE,hE,uE,epsilon);
	calculateVelocityandMomentum(hu,huW,hW,uW,epsilon);
	calculateVelocityandMomentum(hu,huN,hN,uN,epsilon);
	calculateVelocityandMomentum(hu,huS,hS,uS,epsilon);

	//calculate new hvE,hvW,hvN,hvS,vE,vW,vN,vS (momentum and velocity)
	calculateVelocityandMomentum(hv,hvE,hE,vE,epsilon);
	calculateVelocityandMomentum(hv,hvW,hW,vW,epsilon);
	calculateVelocityandMomentum(hv,hvN,hN,vN,epsilon);
	calculateVelocityandMomentum(hv,hvS,hS,vS,epsilon);

	speedOfPropagation(aplus,aminus,bplus,bminus,uE,uW,vS,vN,hE,hW,hS,hN,g,N); //calculate new aplus,aminus,bplus,bminus

	firsthalfstep(HxOne,HxTwo,HxThr,HyOne,HyTwo,HyThr,aplus,aminus,bplus,bminus,huE,huW,huN,huS,hvE,hvW,hvN,hvS,wE,wW,wN,wS,BhalfX,BhalfY,g,N); //calculate HxOne,HxTwo,HxThr,HyOne,HyTwo,HyThr
	secondhalfstep(w,hu,hv,HxOne,HxTwo,HxThr,HyOne,HyTwo,HyThr,B,BX,BY,dt,dx,dy,N); //calculate w,hu,hv

	decreaseWater(w,N); //Decrease to normal waterlevel
/*	
	if (i==10)
	{
		H[10][10]=2.0f;	
	}

	std::cout << "Printing Hx:" << std::endl;
	print(N,Hx);
	std::cout << "Printing Hy:" << std::endl;
	print(N,Hy);
*/
/*
	std::cout << "Printing hE:" << std::endl;
	print(N,hE);
	std::cout << "Printing uE:" << std::endl;
	print(N,uE);

	std::cout << "Printing aplus:" << std::endl;
	print(N,aplus);
	std::cout << "Printing aminus:" << std::endl;
	print(N,aminus);

	std::cout << "Printing HxOne:" << std::endl;
	print(N,HxOne);	
	std::cout << "Printing HxTwo:" << std::endl;
	print(N,HxTwo);	
	std::cout << "Printing HxThr:" << std::endl;
	print(N,HxThr);	
*/
	std::cout << "Printing w:" << std::endl;
	print(N,w);
	}

	std::cout << epsilon << std::endl;
	std::cout << dx << std::endl;


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
		delete [] wE[i];
	}
	delete [] wE;

	for (int i = 0; i < (N+2); i++){
		delete [] wW[i];
	}
	delete [] wW;

	for (int i = 0; i < (N+2); i++){
		delete [] wN[i];
	}
	delete [] wN;

	for (int i = 0; i < (N+2); i++){
		delete [] wS[i];
	}
	delete [] wS;




//
	
	for (int i = 0; i < (N+2); i++){
		delete [] hu[i];
	}
	delete [] hu;

	
	for (int i = 0; i < (N+2); i++){
		delete [] hv[i];
	}
	delete [] hv;

//
	for (int i = 0; i < (N+2); i++){
		delete [] hE[i];
	}
	delete [] hE;

	for (int i = 0; i < (N+2); i++){
		delete [] hW[i];
	}
	delete [] hW;

	for (int i = 0; i < (N+2); i++){
		delete [] hN[i];
	}
	delete [] hN;

	for (int i = 0; i < (N+2); i++){
		delete [] hS[i];
	}
	delete [] hS;

//

	for (int i = 0; i < (N+2); i++){
		delete [] uE[i];
	}
	delete [] uE;

	for (int i = 0; i < (N+2); i++){
		delete [] uW[i];
	}
	delete [] uW;

	for (int i = 0; i < (N+2); i++){
		delete [] uN[i];
	}
	delete [] uN;

	for (int i = 0; i < (N+2); i++){
		delete [] uS[i];
	}
	delete [] uS;

	for (int i = 0; i < (N+2); i++){
		delete [] vE[i];
	}
	delete [] vE;

	for (int i = 0; i < (N+2); i++){
		delete [] vW[i];
	}
	delete [] vW;

	for (int i = 0; i < (N+2); i++){
		delete [] vN[i];
	}
	delete [] vN;

	for (int i = 0; i < (N+2); i++){
		delete [] vS[i];
	}
	delete [] vS;

//


	for (int i = 0; i < (N+2); i++){
		delete [] huE[i];
	}
	delete [] huE;

	for (int i = 0; i < (N+2); i++){
		delete [] huW[i];
	}
	delete [] huW;

	for (int i = 0; i < (N+2); i++){
		delete [] huN[i];
	}
	delete [] huN;

	for (int i = 0; i < (N+2); i++){
		delete [] huS[i];
	}
	delete [] huS;

	for (int i = 0; i < (N+2); i++){
		delete [] hvE[i];
	}
	delete [] hvE;

	for (int i = 0; i < (N+2); i++){
		delete [] hvW[i];
	}
	delete [] hvW;

	for (int i = 0; i < (N+2); i++){
		delete [] hvN[i];
	}
	delete [] hvN;

	for (int i = 0; i < (N+2); i++){
		delete [] hvS[i];
	}
	delete [] hvS;

//

	for (int i = 0; i < (N+2); i++){
		delete [] HxOne[i];
	}
	delete [] HxOne;

	for (int i = 0; i < (N+2); i++){
		delete [] HxTwo[i];
	}
	delete [] HxTwo;

	for (int i = 0; i < (N+2); i++){
		delete [] HxThr[i];
	}
	delete [] HxThr;

	for (int i = 0; i < (N+2); i++){
		delete [] HyOne[i];
	}
	delete [] HyOne;

	for (int i = 0; i < (N+2); i++){
		delete [] HyTwo[i];
	}
	delete [] HyTwo;

	for (int i = 0; i < (N+2); i++){
		delete [] HyThr[i];
	}
	delete [] HyThr;

//
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

	for (int i = 0; i < (N+2); i++){
		delete [] BhalfC[i];
	}
	delete [] BhalfC;

	return 0;
}

