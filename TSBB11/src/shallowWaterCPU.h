/// @file shallowWaterCPU.h
/// @brief Contains the CPU implementation of the shallow water equations.
#pragma once
#ifndef ShallowWaterCPU_H
#define ShallowWaterCPU_H

#include "./common/glm/glm.hpp"
#include <vector>
#include <string>

#define GRAVITY -9.81f

/// @class HeightFieldCPU
/// @brief handles the shallow water simulation on the CPU.
class ShallowWaterCPU
{
public:
	/// @brief Constructor for the ShallowWater.
	///
	/// The constructor for the ShallowWaterCPU.
	/// @param sizeX The unsigned size of the grid in width. Defaults to 6.
	/// @param sizeY The unsigned size of the grid in height. Defaults to 6.
	ShallowWaterCPU(const unsigned int sizeX = 6, const unsigned int sizeY = 6);

	/// @brief Standard Destructor for HeightField
	///
	~ShallowWaterCPU();

	/// @brief Runs the simulation and prints data while doint it. Only used for testing
	///
	///This is a temporary function that is called from the main program. Everything except gridSize can be manipulated here. If grid size is to be manipulated do that when you create the object
	int runDebug();

	/// @brief Run the simulation using assigned timestep
	///
	/// @param dt Timestep to use for this iteration.
	void RunSimulation(const float dt);

private:
	const unsigned int m_sizeX; ///< size of grid. Width
	const unsigned int m_sizeY; ///< size of grid. Height
	enum class VELTYPE {HEIGHT, X_VEL, Y_VEL}; ///< for distinguish what type of data we are working with

	/*the arrays with data*/
	std::vector<float> m_water_height; ///< storage for the water height
	std::vector<float> m_terrain_height; ///< height of the terrain
	std::vector <float> temp; ///< temporary storage. Used for calculations only
	std::vector<float> m_velocity_x; ///< velocity of water, x component
	std::vector<float> m_velocity_y; ///< velocity of water, y component
	std::vector<bool> m_fluid; ///< if cell contains water or not
	std::vector<float> m_fillLevel; ///< how much of the cell is filled in %

	float SumOfArray; ///< storage for sum of array
	float SumDifference; ///< storage for differance of sum
	float OldSumArray; ///< storage for last sumValue
	unsigned long long iter; ///< used for printing

	/// @brief Performs bilinjear interpolation on an given array and a given point
	///
	/// @param array The array we want to perform our interpolation from
	/// @param point_x The non-grid point value in the x component
	/// @param point_y The non-grid point value in the y component
	float bilinjearInterpolation(std::vector<float>& array, float point_x, float point_y);

	/// @brief A helper function to reset the temp array to zero
	///
	void ResetTemp();

	/// @brief Performs advocation of data dependent on type
	///
	/// Moves data along the velocity fields. Depending on the data we want to move, we have to move it differently
	/// @param array the array we want to advect
	/// @param dt the timestep
	/// @param type type of data we have in array
	void Advect(std::vector<float>& array,const float dt, VELTYPE type);

	/// @brief Updates the height of water_height by taking the average of neighbours times dt
	/// 
	/// @param dt the timestep
	void UpdateHeight(const float dt);

	/// @brief Updates the velocities using total_height (water + terrain)
	///
	/// @param dt the timestep
	/// @param total_height water height + terrain height
	void UpdateVelocity(const float dt, std::vector<float>& total_height);

	/// @brief Helperfunction that adds the terrain height to input array
	///
	/// @param array The array we want to add the terrain height data to
	void AddTerrainHeight(std::vector<float>& array);

	/// @brief Computes the flags if a cell contains water or not
	///
	/// @param ground_height the terrain height
	/// @param fluid_height the water height
	void ComputeFluidFlags(std::vector<float>& ground_height, std::vector<float>& fluid_height);

	/// @brief Set the boundry condition to be reflective
	///
	/// @warning This function might not work correctly. It is at the moment not enabled
	void SetReflectBoundary();

	/*functions for printing*/
	/// @brief A helperfunction that prints the array of floats with a message and number of iterations if provided
	///
	/// @param arr A float array containing anything we want to display in the console
	/// @param msg A message we want to display together with the array data
	/// @param iter If provided will also display number of iterations with the data.
	void Print(std::vector<float> arr, std::string msg, int iter) const;

	/// @brief A helperfunction that prints the array of bools with a message and number of iterations if provided
	///
	/// @param arr A bool array containing anything we want to display in the console
	/// @param msg A message we want to display together with the array data
	/// @param iter If provided will also display number of iterations with the data.
	void Print(std::vector<bool> arr, std::string msg, int iter) const;

	/// @brief A helperfunction that prints the total sum of the array. Also prints the difference since last called
	///
	/// @param arr A float array containing what we want to sum up.
	float SumArray(std::vector<float>& arr) const;

	/// @brief A helperfunction that prints a dividor and message
	///
	/// @param start_end Prints what should start the dividor with for message
	/// @param msg the message it self
	/// @param iter if provided prints the number of iterations last
	/// @see Print
	void PrintHelper(std::string start_end, std::string msg, int iter) const;

	/// @brief A helperfunction that prints a singel value at a nice distance from other values
	///
	/// @param value the value to be printed
	/// @see Print
	void PrintNumber(float) const; //cant handle bool for some reason. If change change Bool as well

	

	/*functions for setting all values to a specific value*/
	/// @brief A helperfunction that sets all gridpoints of m_water_height to a specific value
	///
	/// @param value The value we want to set to
	void SetWaterHeight(float value);

	/// @brief A helperfunction that sets all gridpoints of m_terrain_height to a specific value
	///
	/// @param value The value we want to set to
	void SetTerrainHeight(float value);

	/// @brief A helperfunction that sets all gridpoints of m_velocity_x to a specific value
	///
	/// @param value The value we want to set to
	void SetVelocity_X(float value);

	/// @brief A helperfunction that sets all gridpoints of m_velocity_y to a specific value
	///
	/// @param value The value we want to set to
	void SetVelocity_Y(float value);

	/*functions for setting a specific value at a specific place*/
	/// @brief A helperfunction that sets gridpoint (x,y) of m_water_height to a specific value
	///
	/// @param value The value we want to set to
	/// @param x The position in X direction 
	/// @param y The position in Y direction
	void SetWaterHeight(float value, unsigned int x, unsigned int y);

	/// @brief A helperfunction that sets gridpoint (x,y) of m_terrain_height to a specific value
	///
	/// @param value The value we want to set to
	/// @param x The position in X direction 
	/// @param y The position in Y direction
	void SetTerrainHeight(float value, unsigned int x, unsigned int y);

	/// @brief A helperfunction that sets gridpoint (x,y) of m_velocity_x to a specific value
	///
	/// @param value The value we want to set to
	/// @param x The position in X direction 
	/// @param y The position in Y direction
	void SetVelocity_X(float value, unsigned int x, unsigned int y);

	/// @brief A helperfunction that sets gridpoint (x,y) of m_velocity_y to a specific value
	///
	/// @param value The value we want to set to
	/// @param x The position in X direction 
	/// @param y The position in Y direction
	void SetVelocity_Y(float value, unsigned int x, unsigned int y);

	/*functions for setting a specific value to a range*/
	/// @brief A helperfunction that sets a range of gridpoints of m_water_height to a specific value
	///
	/// @param value The value we want to set to
	/// @param from_x The starting position in X direction 
	/// @param to_x The end position in X direction
	/// @param from_y The starting position in Y direction 
	/// @param to_y The end position in Y direction
	void SetWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/// @brief A helperfunction that sets a range of gridpoints of m_terrain_height to a specific value
	///
	/// @param value The value we want to set to
	/// @param from_x The starting position in X direction 
	/// @param to_x The end position in X direction
	/// @param from_y The starting position in Y direction 
	/// @param to_y The end position in Y direction
	void SetTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/// @brief A helperfunction that sets a range of gridpoints of m_velocity_x to a specific value
	///
	/// @param value The value we want to set to
	/// @param from_x The starting position in X direction 
	/// @param to_x The end position in X direction
	/// @param from_y The starting position in Y direction 
	/// @param to_y The end position in Y direction
	void SetVelocity_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/// @brief A helperfunction that sets a range of gridpoints of m_velocity_y to a specific value
	///
	/// @param value The value we want to set to
	/// @param from_x The starting position in X direction 
	/// @param to_x The end position in X direction
	/// @param from_y The starting position in Y direction 
	/// @param to_y The end position in Y direction
	void SetVelocity_Y(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/*functions for adding a value to all positions*/
	/// @brief A helperfunction that adds a value to all gridpoints in m_water_height
	///
	/// @param value The value we want to add to current value
	void AddWaterHeight(float value);

	/// @brief A helperfunction that adds a value to all gridpoints in m_terrain_height
	///
	/// @param value The value we want to add to current value
	void AddTerrainHeight(float value);

	/// @brief A helperfunction that adds a value to all gridpoints in m_velocity_x
	///
	/// @param value The value we want to add to current value
	void AddVelocity_X(float value);

	/// @brief A helperfunction that adds a value to all gridpoints in m_velocity_y
	///
	/// @param value The value we want to add to current value
	void AddVelocity_Y(float value);

	/*functions for adding a value to specific position*/
	/// @brief A helperfunction that adds a value to gridpoint (x,y) in m_water_height
	///
	/// @param value The value we want to add
	/// @param x The position in X direction 
	/// @param y The position in Y direction
	void AddWaterHeight(float value, unsigned int x, unsigned int y);

	/// @brief A helperfunction that adds a value to gridpoint (x,y) in m_terrain_height
	///
	/// @param value The value we want to add
	/// @param x The position in X direction 
	/// @param y The position in Y direction
	void AddTerrainHeight(float value, unsigned int x, unsigned int y);

	/// @brief A helperfunction that adds a value to gridpoint (x,y) in m_velocity_x
	///
	/// @param value The value we want to add
	/// @param x The position in X direction 
	/// @param y The position in Y direction
	void AddVelocity_X(float value, unsigned int x, unsigned int y);

	/// @brief A helperfunction that adds a value to gridpoint (x,y) in m_velocity_y
	///
	/// @param value The value we want to add
	/// @param x The position in X direction 
	/// @param y The position in Y direction
	void AddVelocity_Y(float value, unsigned int x, unsigned int y);

	/*functions for adding a value to a range of values*/
	/// @brief A helperfunction that adds a value to a range of gridpoint in m_water_height
	///
	/// @param value The value we want to add
	/// @param from_x The starting position in X direction 
	/// @param to_x The end position in X direction
	/// @param from_y The starting position in Y direction 
	/// @param to_y The end position in Y direction
	void AddWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/// @brief A helperfunction that adds a value to a range of gridpoint in m_terrain_height
	///
	/// @param value The value we want to add
	/// @param from_x The starting position in X direction 
	/// @param to_x The end position in X direction
	/// @param from_y The starting position in Y direction 
	/// @param to_y The end position in Y direction
	void AddTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/// @brief A helperfunction that adds a value to a range of gridpoint in m_velocity_x
	///
	/// @param value The value we want to add
	/// @param from_x The starting position in X direction 
	/// @param to_x The end position in X direction
	/// @param from_y The starting position in Y direction 
	/// @param to_y The end position in Y direction
	void AddVelocity_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/// @brief A helperfunction that adds a value to a range of gridpoint in m_velocity_y
	///
	/// @param value The value we want to add
	/// @param from_x The starting position in X direction 
	/// @param to_x The end position in X direction
	/// @param from_y The starting position in Y direction 
	/// @param to_y The end position in Y direction
	void AddVelocity_Y(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/*functions for subbing a value to all positions*/
	/// @brief A helperfunction that substracts a value to all gridpoints in m_water_height
	///
	/// Calls the respective Add function with negative value
	/// @param value The value we want to substract
	/// @see AddWaterHeight
	void SubWaterHeight(float value);

	/// @brief A helperfunction that substracts a value to all gridpoints in m_terrain_height
	///
	/// Calls the respective Add function with negative value
	/// @param value The value we want to substract
	/// @see AddTerrainHeight
	void SubTerrainHeight(float value);

	/// @brief A helperfunction that substracts a value to all gridpoints in m_velocity_x
	///
	/// Calls the respective Add function with negative value
	/// @param value The value we want to substract
	/// @see AddVelocity_X
	void SubVelocity_X(float value);

	/// @brief A helperfunction that substracts a value to all gridpoints in m_velocity_y
	///
	/// Calls the respective Add function with negative value
	/// @param value The value we want to substract
	/// @see AddVelocity_Y
	void SubVelocity_Y(float value);

	/*functions for subbing a value to specific position*/
	/// @brief A helperfunction that substracts a value from gridpoint (x,y) in m_water_height
	///
	/// Calls the respective Add function with negative value
	/// @param value The value we want to substract
	/// @param x The position in X direction 
	/// @param y The position in Y direction
	/// @see AddWaterHeight
	void SubWaterHeight(float value, unsigned int x, unsigned int y);

	/// @brief A helperfunction that substracts a value to all gridpoints in m_terrain_height
	///
	/// Calls the respective Add function with negative value
	/// @param value The value we want to substract
	/// @param x The position in X direction 
	/// @param y The position in Y direction
	/// @see AddTerrainHeight
	void SubTerrainHeight(float value, unsigned int x, unsigned int y);

	/// @brief A helperfunction that substracts a value to all gridpoints in m_velocity_x
	///
	/// Calls the respective Add function with negative value
	/// @param value The value we want to substract
	/// @param x The position in X direction 
	/// @param y The position in Y direction
	/// @see AddVelocity_X
	void SubVelocity_X(float value, unsigned int x, unsigned int y);

	/// @brief A helperfunction that substracts a value to all gridpoints in m_velocity_y
	///
	/// Calls the respective Add function with negative value
	/// @param value The value we want to substract
	/// @param x The position in X direction 
	/// @param y The position in Y direction
	/// @see AddVelocity_Y
	void SubVelocity_Y(float value, unsigned int x, unsigned int y);

	/*functions for subbing a value to a range of values*/
	/// @brief A helperfunction that substracts a value from gridpoint (x,y) in m_water_height
	///
	/// Calls the respective Add function with negative value
	/// @param value The value we want to add
	/// @param from_x The starting position in X direction 
	/// @param to_x The end position in X direction
	/// @param from_y The starting position in Y direction 
	/// @param to_y The end position in Y direction
	/// @see AddWaterHeight
	void SubWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/// @brief A helperfunction that substracts a value to all gridpoints in m_terrain_height
	///
	/// Calls the respective Add function with negative value
	/// @param value The value we want to add
	/// @param from_x The starting position in X direction 
	/// @param to_x The end position in X direction
	/// @param from_y The starting position in Y direction 
	/// @param to_y The end position in Y direction
	/// @see AddTerrainHeight
	void SubTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/// @brief A helperfunction that substracts a value to all gridpoints in m_velocity_x
	///
	/// Calls the respective Add function with negative value
	/// @param value The value we want to add
	/// @param from_x The starting position in X direction 
	/// @param to_x The end position in X direction
	/// @param from_y The starting position in Y direction 
	/// @param to_y The end position in Y direction
	/// @see AddVelocity_X
	void SubVelocity_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/// @brief A helperfunction that substracts a value to all gridpoints in m_velocity_y
	///
	/// Calls the respective Add function with negative value
	/// @param value The value we want to add
	/// @param from_x The starting position in X direction 
	/// @param to_x The end position in X direction
	/// @param from_y The starting position in Y direction 
	/// @param to_y The end position in Y direction
	/// @see AddVelocity_Y
	void SubVelocity_Y(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y);

	/*Functions for printing to the console. iter is default -1 if any other value you print that too*/
	/// @brief A helperfunction that prints the m_water_height array to the console
	///
	/// @param iteration if provided also prints iteration number
	/// @see Print
	void PrintWaterHeight(int iteration = -1) const;

	/// @brief A helperfunction that prints the m_terrain_height array to the console
	///
	/// @param iteration if provided also prints iteration number
	/// @see Print
	void PrintTerrainHeight(int iteration = -1) const;

	/// @brief A helperfunction that prints the m_velocity_x array to the console
	///
	/// @param iteration if provided also prints iteration number
	/// @see Print
	void PrintVelocity_X(int iteration = -1) const;

	/// @brief A helperfunction that prints the m_velocity_y array to the console
	///
	/// @param iteration if provided also prints iteration number
	/// @see Print
	void PrintVelocity_Y(int iteration = -1) const;

	/// @brief A helperfunction that prints the m_fluid array to the console
	///
	/// @param iteration if provided also prints iteration number
	/// @see Print
	void PrintWaterBool(int iteration = -1) const;

	/// @brief A helperfunction that prints the m_water_fill array to the console
	///
	/// @param iteration if provided also prints iteration number
	/// @see Print
	void PrintWaterFillLevel(int iteration = -1) const;

	/*functions for printing a specific value at a specific position*/
	/// @brief A helperfunction that prints the value at x,y of the m_terrain_height array to the console
	///
	/// @param x The position in X direction 
	/// @param y The position in Y direction
	/// @see PrintNumber
	void PrintWaterHeight(unsigned int x, unsigned int y) const;

	/// @brief A helperfunction that prints the value at x,y of the m_terrain_height array to the console
	///
	/// @param x The position in X direction 
	/// @param y The position in Y direction
	/// @see PrintNumber
	void PrintTerrainHeight(unsigned int x, unsigned int y) const;

	/// @brief A helperfunction that prints the value at x,y of the m_velocity_x array to the console
	///
	/// @param x The position in X direction 
	/// @param y The position in Y direction
	/// @see PrintNumber
	void PrintVelocity_X(unsigned int x, unsigned int y) const;

	/// @brief A helperfunction that prints the value at x,y of the m_velocity_y array to the console
	///
	/// @param x The position in X direction 
	/// @param y The position in Y direction
	/// @see PrintNumber
	void PrintVelocity_Y(unsigned int x, unsigned int y) const;

	/// @brief A helperfunction that prints the value at x,y of the m_fluid array to the console
	///
	/// @param x The position in X direction 
	/// @param y The position in Y direction
	/// @see PrintNumber
	void PrintWaterBool(unsigned int x, unsigned int y) const;

	/// @brief A helperfunction that prints the value at x,y of the m_water_fill array to the console
	///
	/// @param x The position in X direction 
	/// @param y The position in Y direction
	/// @see PrintNumber
	void PrintWaterFillLevel(unsigned int x, unsigned int y) const;

	/*functions to print values from a range*/
	/// @brief A helperfunction that prints the value at the range from_x to_x and from_y to_y of the m_water_height array to the console
	///
	/// @param from_x The starting position in X direction 
	/// @param to_x The end position in X direction
	/// @param from_y The starting position in Y direction 
	/// @param to_y The end position in Y direction
	/// @see PrintNumber
	void PrintWaterHeight(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const;

	/// @brief A helperfunction that prints the value at the range from_x to_x and from_y to_y of the m_terrain_height array to the console
	///
	/// @param from_x The starting position in X direction 
	/// @param to_x The end position in X direction
	/// @param from_y The starting position in Y direction 
	/// @param to_y The end position in Y direction
	/// @see PrintNumber
	void PrintTerrainHeight(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const;

	/// @brief A helperfunction that prints the value at the range from_x to_x and from_y to_y of the m_velocity_x array to the console
	///
	/// @param from_x The starting position in X direction 
	/// @param to_x The end position in X direction
	/// @param from_y The starting position in Y direction 
	/// @param to_y The end position in Y direction
	/// @see PrintNumber
	void PrintVelocity_X(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const;

	/// @brief A helperfunction that prints the value at the range from_x to_x and from_y to_y of the m_velocity_y array to the console
	///
	/// @param from_x The starting position in X direction 
	/// @param to_x The end position in X direction
	/// @param from_y The starting position in Y direction 
	/// @param to_y The end position in Y direction
	/// @see PrintNumber
	void PrintVelocity_Y(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const;

	/// @brief A helperfunction that prints the value at the range from_x to_x and from_y to_y of the m_fluid array to the console
	///
	/// @param from_x The starting position in X direction 
	/// @param to_x The end position in X direction
	/// @param from_y The starting position in Y direction 
	/// @param to_y The end position in Y direction
	/// @see PrintNumber
	void PrintWaterBool(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const;

	/// @brief A helperfunction that prints the value at the range from_x to_x and from_y to_y of the m_water_fill array to the console
	///
	/// @param from_x The starting position in X direction 
	/// @param to_x The end position in X direction
	/// @param from_y The starting position in Y direction 
	/// @param to_y The end position in Y direction
	/// @see PrintNumber
	void PrintWaterFillLevel(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const;

	/*Helper functions to pause and wait for input*/
	/// @brief A helperfunction that pauses the simulation and waits for inputs to the console
	///
	void Pause() const;

	/// @brief A helperfunction that pauses the simulation and waits for inputs to the console
	///
	/// @param msg A message to print
	void Pause(std::string msg) const;

	/// @brief A helperfunction that prints the sum of the waterheight
	///
	/// @param iter Number of iterations
	void PrintWaterHeightSum(int iter = -1);
};

#endif
