#ifndef FLUIDSOLVER_H
#define FLUIDSOLVER_H

//#define IX(i,j,k) ((i)+(N+2)*(j)+(N+2)*(N+2)*(k))
//#define SWAP(x0, x) {float *tmp=x0;x0=x;x=tmp;}

#include "../../TSBB11/src/common/glm/glm.hpp"
#include <vector>
#include <assert.h>

enum class Boundry{NONE, TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK};

//the voxels
class Voxel
{
public:
	Voxel(): viscosity(1.0f), diffuse(1.0f), boundry(Boundry::NONE) {};
	~Voxel() {};

	float density;
	float prev_density;
	
	float divergence;
	float prev_divergence;
	glm::vec3 velocity;
	glm::vec3 prev_velocity;

	const float viscosity;
	const float diffuse;
	Boundry boundry;
	//dt should not be here because we might want to change it during runtime
};


class NeighbourVoxels
{
public:
	NeighbourVoxels () {};
	~NeighbourVoxels() { delete Up; delete Down; delete Left; delete Right; delete Front; delete Back; delete Origin;};
	Voxel* Up;
	Voxel* Down;
	Voxel* Left;
	Voxel* Right;
	Voxel* Front;
	Voxel* Back;
	Voxel* Origin;
};

class Grid
{
public:
	Grid()
	{
		m_grid = new Voxel**[m_size+2];
		for (unsigned int i = 0; i < m_size+2; i++){
		m_grid[i] = new Voxel*[m_size+2];
		for (unsigned int j = 0; j < m_size+2; j++){
			m_grid[i][j] = new Voxel[m_size+2];
		}
		}
	}
	~Grid()
	{
		for (unsigned int i = 0; i < (m_size+2); i++){
			for (unsigned int j =0; j < (m_size+2); j++){
				delete [] m_grid[i][j];
			}
			delete [] m_grid[i];
		}
		delete [] m_grid;
	}

	Voxel* getVoxel(unsigned int x,unsigned int y,unsigned int z)
	{
		return &m_grid[x][y][z];
	}

	NeighbourVoxels* getNeighbour(unsigned int x, unsigned int y, unsigned int z)
	{
		NeighbourVoxels* vox = new NeighbourVoxels;
		vox->Up = getVoxel(x,y - 1, z);
		vox->Down = getVoxel(x,y + 1, z);
		vox->Left = getVoxel(x -1, y, z);
		vox->Right = getVoxel(x + 1, y, z);
		vox->Front = getVoxel(x, y, z +1);
		vox->Back = getVoxel(x, y, z - 1);
		vox->Origin = getVoxel(x,y,z);
		return vox;
	}
private:
	Voxel*** m_grid;
	unsigned int m_size;
};

class FluidSolver
{
public:
	FluidSolver() {};
	~FluidSolver() {};
private:
	void diffuse_velocity(float dt);
	void diffuse_one_velocity(NeighbourVoxels* vox, float constantData);
	void diffuse_one_density(NeightbourVoxels* vox, float constantData);
	Grid m_grid;
};
























































/*
class fluidsolver
{
public:
	fluidsolver(int N);
	fluidsolver(int height, int width, int depth);
	~fluidsolver() {};

	void set_bnd(int b, float* x);
	void add_source(float *x, float *s);
	void diffuse(int b, float *x, float *x0, float diff);
	void advect (int b, float *d, float *d0, float *u, float *v, float *w);

	void project (float *u, float *v, float *w, float *p, float *div);

	void dens_step(float *x, float *x0, float *u, float *v, float *w, float diff);
	void vel_step(float *u, float *v, float *w, float *u0, float *v0, float *w0, float visc);

	void print(float* v);
private:
	
	std::vector<glm::vec3> _velocity;
	int _height;
	int _width;
	int _depth;
	int _size;

	float _dt;
};
*/
#endif //fluidsolver
