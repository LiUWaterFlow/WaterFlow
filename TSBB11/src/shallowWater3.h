#pragma once
#ifndef SHALLOW_WATER_3_H
#define SHALLOW_WATER_3_H

#include <vector>
#include <string>
#include <iostream>

#define EPSILON 0.01f
#define GRAVITY 9.81f

struct gridPoint{
	float x; //water height over terrain
	float y; //momentum in X
	float z; //momentum in Y
	float w; //terrain height
};

inline
gridPoint operator+(const gridPoint& lhs, const gridPoint& rhs)
{
	gridPoint temp;
	temp.x = lhs.x + rhs.x;
	temp.y = lhs.y + rhs.y;
	temp.z = lhs.z + rhs.z;
	temp.w = lhs.w + rhs.w;
	return temp;
}
inline
gridPoint operator-(const gridPoint& lhs, const gridPoint& rhs)
{
	gridPoint temp;
	temp.x = lhs.x - rhs.x;
	temp.y = lhs.y - rhs.y;
	temp.z = lhs.z - rhs.z;
	temp.w = lhs.w - rhs.w;
	return temp;
}
inline
gridPoint operator*(const gridPoint& lhs, const gridPoint& rhs)
{
	gridPoint temp;
	temp.x = lhs.x*rhs.x;
	temp.y = lhs.y*rhs.y;
	temp.z = lhs.z*rhs.z;
	temp.w = lhs.w*rhs.w;
	return temp;
}
inline
gridPoint operator*(const gridPoint& lhs, const float& rhs)
{
	gridPoint temp;
	temp.x = rhs*lhs.x;
	temp.y = rhs*lhs.y;
	temp.z = rhs*lhs.z;
	temp.w = rhs*lhs.w;
	return temp;
}
inline
gridPoint operator*(const float& lhs, const gridPoint& rhs)
{
	gridPoint temp;
	temp.x = lhs*rhs.x;
	temp.y = lhs*rhs.y;
	temp.z = lhs*rhs.z;
	temp.w = lhs*rhs.w;
	return temp;
}
enum TYPE { WATER, TERRAIN, MOM_X, MOM_Y };
class ShallowWater3
{
public:
	ShallowWater3(const unsigned int sizeX = 6, const unsigned int sizeY = 6);
	~ShallowWater3();

	/*This is a temporary function that is called from the main program
	*Everything except gridSize can be manipulated here. If grid size wants
	*to be manipulated do that in main.cpp*/
	int run();


	/*functions for setting all values to a specific value*/
	void SetWaterHeight(float value);
	void SetTerrainHeight(float value);
	void SetMomentum_X(float value);
	void SetMomentum_Y(float value);
	/*functions for setting a specific value at a specific place*/
	void SetWaterHeight(float value, unsigned int x, unsigned int y);
	void SetTerrainHeight(float value, unsigned int x, unsigned int y);
	void SetMomentum_X(float value, unsigned int x, unsigned int y);
	void SetMomentum_Y(float value, unsigned int x, unsigned int y);
	/*functions for setting a specific value to a range*/
	void SetWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void SetTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void SetMomentum_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void SetMomentum_Y(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/*functions for adding a value to all positions*/
	void AddWaterHeight(float value);
	void AddTerrainHeight(float value);
	void AddMomentum_X(float value);
	void AddMomentum_Y(float value);
	/*functions for adding a value to specific position*/
	void AddWaterHeight(float value, unsigned int x, unsigned int y);
	void AddTerrainHeight(float value, unsigned int x, unsigned int y);
	void AddMomentum_X(float value, unsigned int x, unsigned int y);
	void AddMomentum_Y(float value, unsigned int x, unsigned int y);
	/*functions for adding a value to a range of values*/
	void AddWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void AddTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void AddMomentum_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void AddMomentum_Y(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/*functions for subbing a value to all positions*/
	void SubWaterHeight(float value);
	void SubTerrainHeight(float value);
	void SubMomentum_X(float value);
	void SubMomentum_Y(float value);
	/*functions for subbing a value to specific position*/
	void SubWaterHeight(float value, unsigned int x, unsigned int y);
	void SubTerrainHeight(float value, unsigned int x, unsigned int y);
	void SubMomentum_X(float value, unsigned int x, unsigned int y);
	void SubMomentum_Y(float value, unsigned int x, unsigned int y);
	/*functions for subbing a value to a range of values*/
	void SubWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void SubTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void SubMomentum_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void SubMomentum_Y(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/*Functions for printing to the console. iter is default -1 if any other value you print that too*/
	void PrintWaterHeight(int iteration = -1);
	void PrintTerrainHeight(int iteration = -1);
	void PrintMomentum_X(int iteration = -1);
	void PrintMomentum_Y(int iteration = -1);
	/*functions for printing a specific value at a specific position*/
	void PrintWaterHeight(unsigned int x, unsigned int y);
	void PrintTerrainHeight(unsigned int x, unsigned int y);
	void PrintMomentum_X(unsigned int x, unsigned int y);
	void PrintMomentum_Y(unsigned int x, unsigned int y);
	/*functions to print values from a range*/
	void PrintWaterHeight(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void PrintTerrainHeight(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void PrintMomentum_X(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	void PrintMomentum_Y(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);
	/*Helper functions to pause and wait for input*/
	void Pause() const;
	void Pause(std::string msg);

	/*=======================================================
	* These are special functions that should be removed later
	* They are here so the testcode from ShallowWater2 runs
	*==========================================================*/
	/*set functions*/
	void SetVelocity_X(float value) { SetMomentum_X(value); }
	void SetVelocity_Y(float value) { SetMomentum_Y(value); }
	void SetVelocity_X(float value, unsigned int x, unsigned int y) { SetMomentum_X(value, x, y); }
	void SetVelocity_Y(float value, unsigned int x, unsigned int y) { SetMomentum_Y(value, x, y); }
	void SetVelocity_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
		{ SetMomentum_X(value, from_x, to_x, from_y, to_y); }
	void SetVelocity_Y(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
		{ SetMomentum_Y(value, from_x, to_x, from_y, to_y); }
	/*Add functions*/
	void AddVelocity_X(float value) { AddMomentum_X(value); }
	void AddVelocity_Y(float value) { AddMomentum_Y(value); }
	void AddVelocity_X(float value, unsigned int x, unsigned int y) { AddMomentum_X(value, x, y); }
	void AddVelocity_Y(float value, unsigned int x, unsigned int y) { AddMomentum_Y(value, x, y); }
	void AddVelocity_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
		{ AddMomentum_X(value, from_x, to_x, from_y, to_y); }
	void AddVelocity_Y(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
		{ AddMomentum_Y(value, from_x, to_x, from_y, to_y); }
	/*sub functions*/
	void SubVelocity_X(float value) { SubMomentum_X(value); }
	void SubVelocity_Y(float value) { SubMomentum_Y(value); }
	void SubVelocity_X(float value, unsigned int x, unsigned int y) { SubMomentum_X(value, x, y); }
	void SubVelocity_Y(float value, unsigned int x, unsigned int y) { SubMomentum_Y(value, x, y); }
	void SubVelocity_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
	{ SubMomentum_X(value, from_x, to_x, from_y, to_y); }
	void SubVelocity_Y(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
	{ SubMomentum_Y(value, from_x, to_x, from_y, to_y); }
	/*Print functions*/
	void PrintVelocity_X(int iteration = -1) { PrintMomentum_X(iteration); }
	void PrintVelocity_Y(int iteration = -1) { PrintMomentum_Y(iteration); }
	void PrintVelocity_X(unsigned int x, unsigned int y) { PrintMomentum_X(x, y); }
	void PrintVelocity_Y(unsigned int x, unsigned int y) { PrintMomentum_Y(x, y); }
	void PrintVelocity_X(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
		{ PrintMomentum_X(from_x, to_x, from_y, to_y); }
	void PrintVelocity_Y(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
		{ PrintMomentum_Y(from_x, to_x, from_y, to_y); }

	void PrintWaterBool(int iteration = -1) { std::cout << "Not Working in ShallowWater3" << std::endl; }
	void PrintWaterFillLevel(int iteration = -1) { std::cout << "Not Working in ShallowWater3" << std::endl; }
	void PrintWaterBool(unsigned int x, unsigned int y) { std::cout << "Not Working in ShallowWater3" << std::endl; }
	void PrintWaterFillLevel(unsigned int x, unsigned int y) { std::cout << "Not Working in ShallowWater3" << std::endl; }
	void PrintWaterBool(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) { std::cout << "Not Working in ShallowWater3" << std::endl; }
	void PrintWaterFillLevel(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) { std::cout << "Not Working in ShallowWater3" << std::endl; }

	void PrintWaterHeightSum(int iter = -1);
private:
	const unsigned int m_sizeX;
	const unsigned int m_sizeY;

	gridPoint& grid(unsigned int x, unsigned int y, std::vector<gridPoint>& arr);
	void RunSimulation(const float timestep);

	void fixShore(gridPoint& left, gridPoint& center, gridPoint& right);
	gridPoint SlopeForce(gridPoint& center, gridPoint& north, gridPoint& east, gridPoint& south, gridPoint& west);
	gridPoint HorizontalPotential(gridPoint gp);
	gridPoint VerticalPotential(gridPoint gp);

	void fixBoundry();

	void Print(TYPE type, std::string msg, int iter);
	void PrintNumber(TYPE type, gridPoint& gp);
	void PrintHelper(std::string start_end, std::string msg, int iter);
	float SumArray(TYPE type);

	void ResetTemp();

	std::vector<gridPoint> m_grid;
	std::vector<gridPoint> m_temp;

	float SumOfArray;
	float SumDifference;
	float OldSumArray;
};

#endif //SHALLOW_WATER_3_H