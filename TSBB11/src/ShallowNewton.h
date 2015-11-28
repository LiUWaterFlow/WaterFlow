#pragma once
#ifndef SHALLOWNEWTON_H
#define SHALLOWNEWTON_H

#include <vector>
#include <string>

class ShallowNewton
{
public:
	ShallowNewton(const unsigned int sizeX = 6, const unsigned int sizeY = 6);
	~ShallowNewton();

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
	/*functions for setting a specific value at a specific place*/
	void SetWaterHeight(float value, unsigned int x, unsigned int y);
	void SetTerrainHeight(float value, unsigned int x, unsigned int y);
	void setVelocity_X(float value, unsigned int x, unsigned int y);
	/*functions for setting a specific value to a range*/
	void SetWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void SetTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void setVelocity_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/*functions for adding a value to all positions*/
	void AddWaterHeight(float value);
	void AddTerrainHeight(float value);
	void AddVelocity_X(float value);
	/*functions for adding a value to specific position*/
	void AddWaterHeight(float value, unsigned int x, unsigned int y);
	void AddTerrainHeight(float value, unsigned int x, unsigned int y);
	void AddVelocity_X(float value, unsigned int x, unsigned int y);
	/*functions for adding a value to a range of values*/
	void AddWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void AddTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void AddVelocity_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/*functions for subbing a value to all positions*/
	void SubWaterHeight(float value);
	void SubTerrainHeight(float value);
	void SubVelocity_X(float value);
	/*functions for subbing a value to specific position*/
	void SubWaterHeight(float value, unsigned int x, unsigned int y);
	void SubTerrainHeight(float value, unsigned int x, unsigned int y);
	void SubVelocity_X(float value, unsigned int x, unsigned int y);
	/*functions for subbing a value to a range of values*/
	void SubWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void SubTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void SubVelocity_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/*Functions for printing to the console. iter is default -1 if any other value you print that too*/
	void PrintWaterHeight(int iteration = -1) const;
	void PrintTerrainHeight(int iteration = -1) const;
	void PrintVelocity_X(int iteration = -1) const;
	/*functions for printing a specific value at a specific position*/
	void PrintWaterHeight(unsigned int x, unsigned int y) const;
	void PrintTerrainHeight(unsigned int x, unsigned int y) const;
	void PrintVelocity_X(unsigned int x, unsigned int y) const;
	/*functions to print values from a range*/
	void PrintWaterHeight(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const;
	void PrintTerrainHeight(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const;
	void PrintVelocity_X(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const;
	/*Helper functions to pause and wait for input*/
	void Pause() const;
	void Pause(std::string msg) const;
	void PrintWaterHeightSum(int iter = -1);
private:
	float getHeight(int i, int j, float ourWater, float ourTot);
	void updateSim(float dt);

	/*functions for printing*/
	void Print(std::vector<float> arr, std::string msg, int iter) const;

	float SumArray(std::vector<float>& arr) const;

	void Print(std::vector<bool> arr, std::string msg, int iter) const;
	void PrintHelper(std::string start_end, std::string msg, int iter) const;

	void PrintNumber(float) const; //cant handle bool for some reason. If change change Bool as well

	unsigned int m_sizeX;
	unsigned int m_sizeY;

	std::vector<float> m_water_height;
	std::vector<float> temp;
	std::vector<float> m_velocity_z; //v hastighet i z-led
	std::vector<float> m_terrain_height;

	float SumOfArray;
	float SumDifference;
	float OldSumArray;
};

#endif //SHALLOWNEWTON_H