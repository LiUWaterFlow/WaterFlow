#pragma once
#ifndef FLUIDDATASTRUCTURES
#define FLUIDDATASTRUCTURES

#include <vector>
#include "common\glm\vec3.hpp"

enum CUBEPOS {
	BACK_TOP_LEFT, BACK_TOP_CENTER, BACK_TOP_RIGHT,
	BACK_MID_LEFT, BACK_MID_CENTER, BACK_MID_RIGHT,
	BACK_BOTTOM_LEFT, BACK_BOTTOM_CENTER, BACK_BOTTOM_RIGHT,
	CURRENT_TOP_LEFT, CURRENT_TOP_CENTER, CURRENT_TOP_RIGHT,
	CURRENT_MID_LEFT, CURRENT_MID_CENTER, CURRENT_MID_RIGHT,
	CURRENT_BOTTOM_LEFT, CURRENT_BOTTOM_CENTER, CURRENT_BOTTOM_RIGHT,
	FRONT_TOP_LEFT, FRONT_TOP_CENTER, FRONT_TOP_RIGHT,
	FRONT_MID_LEFT, FRONT_MID_CENTER, FRONT_MID_RIGHT,
	FRONT_BOTTOM_LEFT, FRONT_BOTTOM_CENTER, FRONT_BOTTOM_RIGHT
};
// along x-axis: LEFT-CENTER-RIGHT along y-axis: TOP-MID-BOTTOM along z-axis: BACK-CURRENT-FRONT

//the voxels
struct Voxel
{
	Voxel() : viscosity(1.0f), diffuse(1.0f) {};
	~Voxel() {};

	float density;
	float prev_density;

	float divergence;
	float preassure;
	glm::vec3 velocity;
	glm::vec3 prev_velocity;

	const float viscosity;
	const float diffuse;
};


struct NeighbourVoxels
{
	NeighbourVoxels() {};
	~NeighbourVoxels() {
		voxels.erase(voxels.cbegin(), voxels.cend());
	};

	std::vector<Voxel*> voxels;
};

#endif