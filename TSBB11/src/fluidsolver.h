#ifndef FLUIDSOLVER_H
#define FLUIDSOLVER_H

#include "glm.hpp"
#include <vector>
#include <assert.h>
#include "fluiddatastructures.h"
#include "grid.h"

#ifndef LIN_SOLVE
#define LIN_SOLVE 20
#endif

class FluidSolver
{
public:
	FluidSolver(Voxelgrid* grid, Fluid* fluid) : m_grid(grid), m_fluid(fluid) {};
	~FluidSolver() {};

	void run(float dt);
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

	Voxelgrid* m_grid;
	Fluid* m_fluid;
};

#endif //fluidsolver
