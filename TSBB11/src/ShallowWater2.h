#pragma once
#ifndef SHALLOWWATER2_H
#define SHALLOWWATER2_H

#include "./common/glm/glm.hpp"
#include <vector>

#define GRAVITY -9.81f
class ShallowWater2
{
public:
	ShallowWater2(const unsigned int sizeX = 6, const unsigned int sizeY = 6);
	~ShallowWater2();

	/*This is a temporary function that is called from the main program
	 *Everything except gridSize can be manipulated here. If grid size wants
	 *to be manipulated do that in main.cpp*/
	int run();

	/*Run the simulation*/
	void RunSimulation(const float dt);

	/*functions for setting all values to a specific value*/
	void SetWaterHeight(float value);
	void SetTerrainHeight(float value);
	void setVelocity_X(float value);
	void SetVelocity_Y(float value);
	/*functions for setting a specific value at a specific place*/
	void SetWaterHeight(float value, unsigned int x, unsigned int y);
	void SetTerrainHeight(float value, unsigned int x, unsigned int y);
	void setVelocity_X(float value, unsigned int x, unsigned int y);
	void SetVelocity_Y(float value, unsigned int x, unsigned int y);
	/*functions for setting a specific value to a range*/
	void SetWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void SetTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void setVelocity_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void SetVelocity_Y(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/*functions for adding a value to all positions*/
	void AddWaterHeight(float value);
	void AddTerrainHeight(float value);
	void AddVelocity_X(float value);
	void AddVelocity_Y(float value);
	/*functions for adding a value to specific position*/
	void AddWaterHeight(float value, unsigned int x, unsigned int y);
	void AddTerrainHeight(float value, unsigned int x, unsigned int y);
	void AddVelocity_X(float value, unsigned int x, unsigned int y);
	void AddVelocity_Y(float value, unsigned int x, unsigned int y);
	/*functions for adding a value to a range of values*/
	void AddWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void AddTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void AddVelocity_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void AddVelocity_Y(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/*functions for subbing a value to all positions*/
	void SubWaterHeight(float value);
	void SubTerrainHeight(float value);
	void SubVelocity_X(float value);
	void SubVelocity_Y(float value);
	/*functions for subbing a value to specific position*/
	void SubWaterHeight(float value, unsigned int x, unsigned int y);
	void SubTerrainHeight(float value, unsigned int x, unsigned int y);
	void SubVelocity_X(float value, unsigned int x, unsigned int y);
	void SubVelocity_Y(float value, unsigned int x, unsigned int y);
	/*functions for subbing a value to a range of values*/
	void SubWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void SubTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void SubVelocity_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void SubVelocity_Y(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/*Functions for printing to the console. iter is default -1 if any other value you print that too*/
	void PrintWaterHeight(int iteration = -1) const;
	void PrintTerrainHeight(int iteration = -1) const;
	void PrintVelocity_X(int iteration = -1) const;
	void PrintVelocity_Y(int iteration = -1) const;
	void PrintWaterBool(int iteration = -1) const;
	void PrintWaterFillLevel(int iteration = -1) const;
	/*functions for printing a specific value at a specific position*/
	void PrintWaterHeight(unsigned int x, unsigned int y) const;
	void PrintTerrainHeight(unsigned int x, unsigned int y) const;
	void PrintVelocity_X(unsigned int x, unsigned int y) const;
	void PrintVelocity_Y(unsigned int x, unsigned int y) const;
	void PrintWaterBool(unsigned int x, unsigned int y) const;
	void PrintWaterFillLevel(unsigned int x, unsigned int y) const;
	/*functions to print values from a range*/
	void PrintWaterHeight(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const;
	void PrintTerrainHeight(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const;
	void PrintVelocity_X(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const;
	void PrintVelocity_Y(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const;
	void PrintWaterBool(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const;
	void PrintWaterFillLevel(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const;
	/*Helper functions to pause and wait for input*/
	void Pause() const;
	void Pause(std::string msg) const;

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

	/*functions for printing*/
	template <typename T>
	void Print(std::vector<T> arr, std::string msg, int iter) const;
	void PrintHelper(std::string start_end, std::string msg, int iter) const;
	template <typename T>
	void PrintNumber(T& value) const; //cant handle bool for some reason. If change change Bool as well

	/*the arrays with data*/
	std::vector<float> m_water_height;
	std::vector<float> m_terrain_height;
	std::vector <float> temp;
	std::vector<float> m_velocity_x;
	std::vector<float> m_velocity_y;
	std::vector<bool> m_fluid;
	std::vector<float> m_fillLevel;

};

#endif