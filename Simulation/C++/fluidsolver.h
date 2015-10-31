#ifndef FLUIDSOLVER_H
#define FLUIDSOLVER_H

#include "../../TSBB11/src/common/glm/glm.hpp"
#include <vector>
#include <assert.h>

enum CUBEPOS{BACK_TOP_LEFT, BACK_TOP_CENTER, BACK_TOP_RIGHT,
			BACK_MID_LEFT, BACK_MID_CENTER, BACK_MID_RIGHT,
			BACK_BOTTOM_LEFT, BACK_BOTTOM_CENTER, BACK_BOTTOM_RIGHT,
			CURRENT_TOP_LEFT, CURRENT_TOP_CENTER, CURRENT_TOP_RIGHT,
			CURRENT_MID_LEFT, CURRENT_MID_CENTER, CURRENT_MID_RIGHT,
			CURRENT_BOTTOM_LEFT, CURRENT_BOTTOM_CENTER, CURRENT_BOTTOM_RIGHT,
			FRONT_TOP_LEFT, FRONT_TOP_CENTER, FRONT_TOP_RIGHT,
			FRONT_MID_LEFT, FRONT_MID_CENTER, FRONT_MID_RIGHT,
			FRONT_BOTTOM_LEFT, FRONT_BOTTOM_CENTER, FRONT_BOTTOM_RIGHT};
// along x-axis: LEFT-CENTER-RIGHT along y-axis: TOP-MID-BOTTOM along z-axis: BACK-CURRENT-FRONT

//the voxels
class Voxel
{
public:
	Voxel(): viscosity(1.0f), diffuse(1.0f) {};
	~Voxel() {};

	float density;
	float prev_density;
	
	float divergence;
	float prev_divergence;
	glm::vec3 velocity;
	glm::vec3 prev_velocity;

	const float viscosity;
	const float diffuse;
	//dt should not be here because we might want to change it during runtime
};


class NeighbourVoxels
{
public:
	NeighbourVoxels () {};
	~NeighbourVoxels() {
		voxels.erase(voxels.cbegin(), voxels.cend());
	};
		
	std::vector<Voxel*> voxels;
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

	NeighbourVoxels getNeighbour(unsigned int x, unsigned int y, unsigned int z)
	{
		NeighbourVoxels vox;
		vox.voxels[CUBEPOS::BACK_TOP_LEFT] = getVoxel(x-1,y-1,z-1);
		vox.voxels[CUBEPOS::BACK_TOP_CENTER] = getVoxel(x,y-1,z-1);
		vox.voxels[CUBEPOS::BACK_TOP_RIGHT] = getVoxel(x+1,y-1,z-1);

		vox.voxels[CUBEPOS::BACK_MID_LEFT] = getVoxel(x-1,y,z-1);
		vox.voxels[CUBEPOS::BACK_MID_CENTER] = getVoxel(x,y,z-1);
		vox.voxels[CUBEPOS::BACK_MID_RIGHT] = getVoxel(x+1,y,z-1);

		vox.voxels[CUBEPOS::BACK_BOTTOM_LEFT] = getVoxel(x-1,y+1,z-1);
		vox.voxels[CUBEPOS::BACK_BOTTOM_CENTER] = getVoxel(x,y+1,z-1);
		vox.voxels[CUBEPOS::BACK_BOTTOM_RIGHT] = getVoxel(x+1,y+1,z-1);

		vox.voxels[CUBEPOS::CURRENT_TOP_LEFT] = getVoxel(x-1,y-1,z);
		vox.voxels[CUBEPOS::CURRENT_TOP_CENTER] = getVoxel(x,y-1,z);
		vox.voxels[CUBEPOS::CURRENT_TOP_RIGHT] = getVoxel(x+1,y-1,z);

		vox.voxels[CUBEPOS::CURRENT_MID_LEFT] = getVoxel(x-1,y,z);
		vox.voxels[CUBEPOS::CURRENT_MID_CENTER] = getVoxel(x,y,z);
		vox.voxels[CUBEPOS::CURRENT_MID_RIGHT] = getVoxel(x+1,y,z);

		vox.voxels[CUBEPOS::CURRENT_BOTTOM_LEFT] = getVoxel(x-1,y+1,z);
		vox.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER] = getVoxel(x,y+1,z);
		vox.voxels[CUBEPOS::CURRENT_BOTTOM_RIGHT] = getVoxel(x+1,y+1,z);

		vox.voxels[CUBEPOS::FRONT_TOP_LEFT] = getVoxel(x-1,y-1,z+1);
		vox.voxels[CUBEPOS::FRONT_TOP_CENTER] = getVoxel(x,y-1,z+1);
		vox.voxels[CUBEPOS::FRONT_TOP_RIGHT] = getVoxel(x+1,y-1,z+1);

		vox.voxels[CUBEPOS::FRONT_MID_LEFT] = getVoxel(x-1,y,z+1);
		vox.voxels[CUBEPOS::FRONT_MID_CENTER] = getVoxel(x,y,z+1);
		vox.voxels[CUBEPOS::FRONT_MID_RIGHT] = getVoxel(x+1,y,z+1);

		vox.voxels[CUBEPOS::FRONT_BOTTOM_LEFT] = getVoxel(x-1,y+1,z+1);
		vox.voxels[CUBEPOS::FRONT_BOTTOM_CENTER] = getVoxel(x,y+1,z+1);
		vox.voxels[CUBEPOS::FRONT_BOTTOM_RIGHT] = getVoxel(x+1,y+1,z+1);
		
		return vox;
	}
	unsigned int XLength;
	unsigned int YLength;
	unsigned int ZLength;
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
	void diffuse_density(float dt);
	void diffuse_one_velocity(NeighbourVoxels& vox, float constantData);
	void diffuse_one_density(NeighbourVoxels& vox, float constantData);

	void advect_velocity(float dt);
	void advect_density(float dt);
	void advect_core_function(float someconstant, glm::ivec3 gridPosition, glm::ivec3 &prev_gridPosition, glm::vec3 &pointPosition, const glm::vec3& midVelocity);
	void advect_one_velocity(glm::ivec3 prev_grid_position, glm::vec3 point_position, Voxel* currentVox, float constantData);
	void advect_one_density(glm::ivec3 prev_grid_position, glm::vec3 point_position, Voxel* currentVox, float constantData);

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
