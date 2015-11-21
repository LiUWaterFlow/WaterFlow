#pragma once
#ifndef SHALLOWWATER2_H
#define SHALLOWWATER2_H

#include "../../../TSBB11/src/common/glm/glm.hpp"
#include <vector>

#define GRAVITY -9.81f
class ShallowWater2
{
public:
	ShallowWater2(const unsigned int sizeX = 6, const unsigned int sizeY = 6);
	~ShallowWater2();

	void RunSimulation(const float dt);

private:
	const unsigned int m_sizeX;
	const unsigned int m_sizeY;

	enum class VELTYPE {HEIGHT, X_VEL, Y_VEL};

	float bilinjearInterpolation(std::vector<float>& array, float point_x, float point_y);

	void Advect(std::vector<float>& array,const float dt, VELTYPE type);
	void UpdateHeight(const float dt);
	void UpdateVelocity(const float dt, std::vector<float>& total_height);

	void AddTerrainHeight(std::vector<float>& array);
	void ComputeFluidFlags(std::vector<float>& fluidDepth, std::vector<float>& total_depth);

	void SetReflectBoundary();

	std::vector<float> m_water_height;
	std::vector<float> m_terrain_height;
	std::vector <float> temp;
	std::vector<float> m_velocity_x;
	std::vector<float> m_velocity_y;
	std::vector<bool> m_fluid;
	std::vector<float> m_fillLevel;

};

#endif