#pragma once
#ifndef FLUIDDATASTRUCTURES
#define FLUIDDATASTRUCTURES

#include <vector>
#include "glm.hpp"

enum CUBEPOS {
	FAR_TOP_LEFT, FAR_TOP_CENTER, FAR_TOP_RIGHT,
	FAR_MID_LEFT, FAR_MID_CENTER, FAR_MID_RIGHT,
	FAR_BOTTOM_LEFT, FAR_BOTTOM_CENTER, FAR_BOTTOM_RIGHT,
	CURRENT_TOP_LEFT, CURRENT_TOP_CENTER, CURRENT_TOP_RIGHT,
	CURRENT_MID_LEFT, CURRENT_MID_CENTER, CURRENT_MID_RIGHT,
	CURRENT_BOTTOM_LEFT, CURRENT_BOTTOM_CENTER, CURRENT_BOTTOM_RIGHT,
	NEAR_TOP_LEFT, NEAR_TOP_CENTER, NEAR_TOP_RIGHT,
	NEAR_MID_LEFT, NEAR_MID_CENTER, NEAR_MID_RIGHT,
	NEAR_BOTTOM_LEFT, NEAR_BOTTOM_CENTER, NEAR_BOTTOM_RIGHT
};
// along x-axis: LEFT-CENTER-RIGHT along y-axis: TOP-MID-BOTTOM along z-axis: FAR-CURRENT-NEAR

//the voxels
struct voxel
{
	voxel() : density(0), prev_density(0), preassure(0), velocity(glm::vec3(0)), prev_velocity(glm::vec3(0)), viscosity(1.0f), diffuse(1.0f){};
	~voxel() {};

	float density;
	float prev_density;

	float divergence;
	float preassure;
	glm::vec3 velocity;
	glm::vec3 prev_velocity;

	int16_t x;
	int16_t y;
	int16_t z;
	bool filled;
	float a;
	float b;

	const float viscosity;
	const float diffuse;
};


struct NeighbourVoxels
{
	NeighbourVoxels() {};
	~NeighbourVoxels() {
		voxels.erase(voxels.cbegin(), voxels.cend());
	};

	std::vector<voxel*> voxels;
};

#endif
