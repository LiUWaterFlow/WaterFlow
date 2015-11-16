#include "shallowWater.h"

#include "Utilities.h"

#include <iostream>
#include <iomanip> 



shallowWater::shallowWater(GLuint gridSize) {
	N = gridSize;
	g = 9.8f;
	dt = 0.01f;
	dx = 1.0f;
	dy = 1.0f;

	init();

	oneArray(N, H);
	zeroArray(N, U);
	zeroArray(N, V);
	zeroArray(N, Hx);
	zeroArray(N, Ux);
	zeroArray(N, Vx);
	zeroArray(N, Hy);
	zeroArray(N, Uy);
	zeroArray(N, Vy);

	// For testing set drop
	for (size_t i = 0; i < 5; i++) {
		for (size_t j = 0; j < 11; j++) {
			H[i][j] = 0.000001f;
		}
	}
	
}

void shallowWater::init() {
	H = new float*[N + 2];
	for (int i = 0; i < N + 2; i++) {
		H[i] = new float[N + 2];
	}

	U = new float*[N + 2];
	for (int i = 0; i < N + 2; i++) {
		U[i] = new float[N + 2];
	}

	V = new float*[N + 2];
	for (int i = 0; i < N + 2; i++) {
		V[i] = new float[N + 2];
	}

	Hx = new float*[N + 2];
	for (int i = 0; i < N + 2; i++) {
		Hx[i] = new float[N + 2];
	}

	Ux = new float*[N + 2];
	for (int i = 0; i < N + 2; i++) {
		Ux[i] = new float[N + 2];
	}

	Vx = new float*[N + 2];
	for (int i = 0; i < N + 2; i++) {
		Vx[i] = new float[N + 2];
	}

	Hy = new float*[N + 2];
	for (int i = 0; i < N + 2; i++) {
		Hy[i] = new float[N + 2];
	}

	Uy = new float*[N + 2];
	for (int i = 0; i < N + 2; i++) {
		Uy[i] = new float[N + 2];
	}

	Vy = new float*[N + 2];
	for (int i = 0; i < N + 2; i++) {
		Vy[i] = new float[N + 2];
	}
}

// Set the boundary conditions
void shallowWater::set_bnd(int N, int b, float** x) {
	for (int i = 0; i <= (N + 1); i++) {
		//goes on the rand!
		//b = 1 means forces in x-axis
		x[0][i] = (b == 1 ? -x[1][i] : x[1][i]);
		x[N + 1][i] = (b == 1 ? -x[N][i] : x[N][i]);
		//b == 2 means forces in y-axis
		x[i][0] = (b == 2 ? -x[i][1] : x[i][1]);
		x[i][N + 1] = (b == 2 ? -x[i][N] : x[i][N]);
	}
}

void shallowWater::firsthalfstep(float** H, float** U, float** V, float** Hx, float** Ux, float** Vx, float** Hy, float** Uy, float** Vy, float dt, float dx, float dy, float g, int N) {
	for (int i = 0; i <= N; i++) {
		for (int j = 0; j <= (N - 1); j++) {
			Hx[i][j] = (H[i + 1][j + 1] + H[i][j + 1]) / 2 - dt / (2 * dx)*(U[i + 1][j + 1] - U[i][j + 1]);
			Ux[i][j] = (U[i + 1][j + 1] + U[i][j + 1]) / 2 - dt / (2 * dx)*((U[i + 1][j + 1] * U[i + 1][j + 1] / H[i + 1][j + 1] + g / 2 * H[i + 1][j + 1] * H[i + 1][j + 1]) - (U[i][j + 1] * U[i][j + 1] / H[i][j + 1] + g / 2 * H[i][j + 1] * H[i][j + 1]));
			Vx[i][j] = (V[i + 1][j + 1] + V[i][j + 1]) / 2 - dt / (2 * dx)*((U[i + 1][j + 1] * V[i + 1][j + 1] / H[i + 1][j + 1]) - (U[i][j + 1] * V[i][j + 1] / H[i][j + 1]));
		}
	}

	for (int i = 0; i <= (N - 1); i++) {
		for (int j = 0; j <= N; j++) {
			Hy[i][j] = (H[i + 1][j + 1] + H[i + 1][j]) / 2 - dt / (2 * dy)*(V[i + 1][j + 1] - V[i + 1][j]);
			Uy[i][j] = (U[i + 1][j + 1] + U[i + 1][j]) / 2 - dt / (2 * dy)*((V[i + 1][j + 1] * U[i + 1][j + 1] / H[i + 1][j + 1]) - (V[i + 1][j] * U[i + 1][j] / H[i + 1][j]));
			Vy[i][j] = (V[i + 1][j + 1] + V[i + 1][j]) / 2 - dt / (2 * dy)*((V[i + 1][j + 1] * V[i + 1][j + 1] / H[i + 1][j + 1] + g / 2 * H[i + 1][j + 1] * H[i + 1][j + 1]) - (V[i + 1][j] * V[i + 1][j] / H[i + 1][j] + g / 2 * H[i + 1][j] * H[i + 1][j]));
		}
	}
}

void shallowWater::secondhalfstep(float** H, float** U, float** V, float** Hx, float** Ux, float** Vx, float** Hy, float** Uy, float** Vy, float dt, float dx, float dy, float g, int N) {
	for (int i = 1; i <= N; i++) {
		for (int j = 1; j <= N; j++) {
			H[i][j] = H[i][j] - (dt / dx)*(Ux[i][j - 1] - Ux[i - 1][j - 1]) - (dt / dy)*(Vy[i - 1][j] - Vy[i - 1][j - 1]);
			U[i][j] = U[i][j] - (dt / dx)*((Ux[i][j - 1] * Ux[i][j - 1] / Hx[i][j - 1] + g / 2 * Hx[i][j - 1] * Hx[i][j - 1]) - (Ux[i - 1][j - 1] * Ux[i - 1][j - 1] / Hx[i - 1][j - 1] + g / 2 * Hx[i - 1][j - 1] * Hx[i - 1][j - 1])) - (dt / dy)*((Vy[i - 1][j] * Uy[i - 1][j] / Hy[i - 1][j]) - (Vy[i - 1][j - 1] * Uy[i - 1][j - 1] / Hy[i - 1][j - 1]));
			V[i][j] = V[i][j] - (dt / dx)*((Ux[i][j - 1] * Vx[i][j - 1] / Hx[i][j - 1]) - (Ux[i - 1][j - 1] * Vx[i - 1][j - 1] / Hx[i - 1][j - 1])) - (dt / dy)*((Vy[i - 1][j] * Vy[i - 1][j] / Hy[i - 1][j] + g / 2 * Hy[i - 1][j] * Hy[i - 1][j]) - (Vy[i - 1][j - 1] * Vy[i - 1][j - 1] / Hy[i - 1][j - 1] + g / 2 * Hy[i - 1][j - 1] * Hy[i - 1][j - 1]));
		}
	}
}

void shallowWater::run() {
	set_bnd(N, 0, H);
	set_bnd(N, 1, U);
	set_bnd(N, 2, V);

	firsthalfstep(H, U, V, Hx, Ux, Vx, Hy, Uy, Vy, dt, dx, dy, g, N);
	secondhalfstep(H, U, V, Hx, Ux, Vx, Hy, Uy, Vy, dt, dx, dy, g, N);
}

void shallowWater::printH() {
	print(N, H);
}

shallowWater::~shallowWater() {
	for (int i = 0; i < (N + 2); i++) {
		delete[] H[i];
	}
	delete[] H;

	for (int i = 0; i < (N + 2); i++) {
		delete[] U[i];
	}
	delete[] U;

	for (int i = 0; i < (N + 2); i++) {
		delete[] V[i];
	}
	delete[] V;

	for (int i = 0; i < (N + 2); i++) {
		delete[] Hx[i];
	}
	delete[] Hx;

	for (int i = 0; i < (N + 2); i++) {
		delete[] Ux[i];
	}
	delete[] Ux;

	for (int i = 0; i < (N + 2); i++) {
		delete[] Vx[i];
	}
	delete[] Vx;

	for (int i = 0; i < (N + 2); i++) {
		delete[] Hy[i];
	}
	delete[] Hy;

	for (int i = 0; i < (N + 2); i++) {
		delete[] Uy[i];
	}
	delete[] Uy;

	for (int i = 0; i < (N + 2); i++) {
		delete[] Vy[i];
	}
	delete[] Vy;
}

