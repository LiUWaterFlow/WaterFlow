#ifndef FLUIDSOLVER_H
#define FLUIDSOLVER_H

#include "glm.hpp"
#include <vector>
#include <assert.h>
#include "fluiddatastructures.h"
#include "voxelgrid.h"

#ifndef LIN_SOLVE
#define LIN_SOLVE 20
#endif

/*
class Grid
{
public:
	Grid()
	{
		m_grid = new voxel**[m_size+2];
		for (unsigned int i = 0; i < m_size+2; i++)
		{
			m_grid[i] = new voxel*[m_size+2];
			for (unsigned int j = 0; j < m_size+2; j++)
			{
				m_grid[i][j] = new voxel[m_size+2];
			}
		}
	}
	~Grid()
	{
		for (unsigned int i = 0; i < (m_size+2); i++)
		{
			for (unsigned int j =0; j < (m_size+2); j++)
			{
				delete [] m_grid[i][j];
			}
			delete [] m_grid[i];
		}
		delete [] m_grid;
	}

	std::vector<voxel*> getBorderTop() { std::vector<voxel*> temp; return temp; }
	std::vector<voxel*> getBorderBottom() { std::vector<voxel*> temp; return temp; }
	std::vector<voxel*> getBorderLeft() { std::vector<voxel*> temp; return temp; }
	std::vector<voxel*> getBorderRight() { std::vector<voxel*> temp; return temp; }
	std::vector<voxel*> getBorderFront() { std::vector<voxel*> temp; return temp; }
	std::vector<voxel*> getBorderBack() { std::vector<voxel*> temp; return temp; }

	voxel* getVoxel(unsigned int x,unsigned int y,unsigned int z)
	{
		return &m_grid[x][y][z];
	}

	voxel* getVoxel(glm::ivec3 xyz)
	{
		return &m_grid[xyz.x][xyz.y][xyz.z];
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

	NeighbourVoxels getNeighbour(voxel* it) { return NeighbourVoxels(); }
	unsigned int XLength;
	unsigned int YLength;
	unsigned int ZLength;
private:
	voxel*** m_grid;
	unsigned int m_size;
};
*/

class FluidSolver
{
public:
	FluidSolver() {};
	~FluidSolver() {};
private:
	void dens_step(float dt);
	void velocity_step(float dt);

	void diffuse_velocity(float dt);
	void diffuse_density(float dt);
	template <typename T>
	void linjear_solve_helper(float constantData, T& current_center, const T& prev_center, const T& c_left,
		const T& c_right, const T& c_above, const T& c_below, const T& c_near, const T& c_far);
	void diffuse_one_velocity(float constantData, NeighbourVoxels& vox);
	void diffuse_one_density(float constantData, NeighbourVoxels& vox);

	void advect_velocity(float dt);
	void advect_density(float dt);
	void advect_core_function(float someconstant, glm::ivec3 &prev_gridPosition, glm::ivec3 gridPosition, glm::vec3 &pointPosition, const glm::vec3& midVelocity);
	void advect_one_velocity(float constantData, glm::ivec3 prev_grid_position, glm::vec3 point_position, voxel* currentVox);
	void advect_one_density(float constantData, glm::ivec3 prev_grid_position, glm::vec3 point_position, voxel* currentVox);

	template <typename T>
	void advect_helper(glm::vec3 point_position, glm::vec3 prev_grid_position,
		T& searched, const T& prev_c_mid_center, const T& prev_near_mid_center, const T& prev_c_top_center,
		const T& prev_near_top_center, const T& prev_c_mid_right, const T& prev_near_mid_right, const T& prev_c_top_right,
		const T& prev_near_top_right);

	void project_velocity(float dt);

	void force_boundries_velocity();
	void force_boundries_density();
	void force_boundries_preassure();
	void force_boundries_divergence();

	Voxelgrid m_grid;
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
