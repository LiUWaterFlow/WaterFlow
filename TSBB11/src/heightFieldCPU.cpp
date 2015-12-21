#include "HeightFieldCPU.h"
#include "GL_utilities.h"
#include "Utilities.h"
#include <algorithm>
#include <stdio.h>
#include <valarray>
#include <inttypes.h>
#include "glm.hpp"
#include <iostream>
#include <iomanip>

int HeightFieldCPU::runDebug()
{
	/*Start by setting data*/
	AddTerrainHeight(0.0f);
	AddVelocity_Z(0);

	//Add Water
	AddWaterHeight(5, m_sizeX/2, m_sizeY/2);

	Pause("Running HeightFieldCPU");
	for (unsigned int i = 0; i < 100000; i++) //maybe run the simulation 5 times
	{
		RunSimulation(0.05f);
	}
	
	Pause("Last Pause before termination");

	return 1; //then end it here
}

float HeightFieldCPU::getHeight(int i, int j, float ourWater, float ourTot) {
	if (i < 0 || j < 0 || i > m_sizeX - 1 || j > m_sizeY- 1){
		return 0.0f;
	}

	i = glm::clamp(i, 0, (int)m_sizeX - 1);
	j = glm::clamp(j, 0, (int)m_sizeY - 1);

	float theirTerr = m_terrain_height[i + j*m_sizeX];
	float theirWater = m_water_height[i + j*m_sizeX];
	float theirTot = theirTerr + theirWater;
	float diff = theirTot - ourTot;

	return glm::clamp(diff, -ourWater / 4.0f, theirWater / 4.0f);
}

void HeightFieldCPU::RunSimulation(float dt) {

	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
		//Change these to clamps.
			int offset = (x + y*m_sizeX);
			float c2 = 25.0f;
			float h2 = 4.0f;

			float ourWater = m_water_height[offset];
			float ourTotal = ourWater + m_terrain_height[offset];
			float u_east = getHeight((x + 1), y, ourWater,ourTotal);
			float u_west = getHeight((x - 1), y, ourWater,ourTotal);
			float u_south = getHeight(x, (y - 1), ourWater,ourTotal);
			float u_north = getHeight(x, (y + 1), ourWater,ourTotal);

			float f = c2 / h2*(u_west + u_east + u_south + u_north);

			float vel = f*dt;

			m_velocity_z[offset] = vel;// *0.9999995f
			temp[offset] = glm::max((ourWater + vel * dt), 0.0f);
		}
	}
	std::swap(m_water_height, temp);

}

float HeightFieldCPU::SumArray(std::vector<float>& arr) const
{
	float sum = 0.0f;
	for (unsigned int x = 1; x < m_sizeX - 1; x++)
	{
		for (unsigned int y = 1; y < m_sizeY - 1; y++)
		{
			const unsigned int index = x + y * m_sizeX;
			sum += arr[index];
		}
	}
	return sum;
}
/*
*=========================================================
*Print function and helper print function
*=========================================================
*/
void HeightFieldCPU::Print(std::vector<float> arr, std::string msg, int iter) const
{
	PrintHelper("BEGIN", msg, iter);
	for (unsigned int i = 0; i < m_sizeX; i++)
	{
		for (unsigned int j = 0; j < m_sizeY; j++)
		{

			const unsigned int index = i + j*m_sizeX;
			PrintNumber(arr.at(index));
		}
		std::cout << "\n";
	}
	PrintHelper("END", msg, iter);
	std::cout << std::flush;
}

void HeightFieldCPU::Print(std::vector<bool> arr, std::string msg, int iter) const
{
	PrintHelper("BEGIN", msg, iter);
	for (unsigned int i = 0; i < m_sizeX; i++)
	{
		for (unsigned int j = 0; j < m_sizeY; j++)
		{

			const unsigned int index = i + j*m_sizeX;
			PrintNumber(arr.at(index));
		}
		std::cout << "\n";
	}
	PrintHelper("END", msg, iter);
	std::cout << std::flush;
}


void HeightFieldCPU::PrintNumber(float value) const
{
	std::cout << std::fixed << std::setw(7) << std::setprecision(3) << value << " ";
}
void HeightFieldCPU::PrintHelper(std::string start_end, std::string msg, int iter) const
{
	if (iter != -1) {
		std::cout << "========== " << start_end << " " << msg << " " << iter << " ==========\n\n";
	}
	else {
		std::cout << "========== " << start_end << " " << msg << " ==========\n\n";
	}
}
void HeightFieldCPU::Pause() const
{
	std::getchar();
}
void HeightFieldCPU::Pause(std::string msg) const
{
	std::cout << msg << std::endl;
	std::getchar();
}

void HeightFieldCPU::PrintWaterHeightSum(int iter)
{
	SumOfArray = SumArray(m_water_height);
	SumDifference = SumOfArray / OldSumArray - 1;
	OldSumArray = SumOfArray;

	std::cout << "Water Height Summation: " << SumOfArray;
	if (iter != -1)
	{
		std::cout << " Iteration: " << iter;
	}
	std::cout << " Percentage difference: " << SumDifference << " \n" << std::endl;

}
/*
*=========================================================
*Print functions showing number of iterations
*No iteration input gives no numbering
*=========================================================
*/
void HeightFieldCPU::PrintWaterHeight(int iteration) const
{
	Print(m_water_height, "WATER_HEIGHT", iteration);
}
void HeightFieldCPU::PrintTerrainHeight(int iteration) const
{
	Print(m_terrain_height, "TERRAIN_HEIGHT", iteration);
}
void HeightFieldCPU::PrintVelocity_Z(int iteration) const
{
	Print(m_velocity_z, "VELOCITY_X", iteration);
}

/*
*=========================================================
*Print functions based on Index
*=========================================================
*Warning: Bool is strange, needed to move PrintNumber thingie over here
*/
void HeightFieldCPU::PrintWaterHeight(unsigned int x, unsigned int y) const
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	PrintNumber(m_water_height.at(index));
	std::cout << std::flush;
}
void HeightFieldCPU::PrintTerrainHeight(unsigned int x, unsigned int y) const
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	PrintNumber(m_terrain_height.at(index));
	std::cout << std::flush;
}
void HeightFieldCPU::PrintVelocity_Z(unsigned int x, unsigned int y) const
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	PrintNumber(m_velocity_z.at(index));
	std::cout << std::flush;
}

/*
*=========================================================
*Print functions based on RANGE
*=========================================================
*
*/
void HeightFieldCPU::PrintWaterHeight(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			PrintNumber(m_water_height.at(index));
		}
	}
	std::cout << std::flush;
}
void HeightFieldCPU::PrintTerrainHeight(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			PrintNumber(m_terrain_height.at(index));
		}
	}
	std::cout << std::flush;
}
void HeightFieldCPU::PrintVelocity_Z(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			PrintNumber(m_velocity_z.at(index));
		}
	}
	std::cout << std::flush;
}
/*
*=========================================================
*Set functions for all positions with value
*=========================================================
*
*/
void HeightFieldCPU::SetWaterHeight(float value)
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_water_height.at(index) = value;
		}
	}
}
void HeightFieldCPU::SetTerrainHeight(float value)
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_terrain_height.at(index) = value;
		}
	}
}
void HeightFieldCPU::SetVelocity_Z(float value)
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_velocity_z.at(index) = value;
		}
	}
}
/*
*=========================================================
*Set functions for specified position with value
*=========================================================
*
*/
void HeightFieldCPU::SetWaterHeight(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	m_water_height.at(index) = value;
}
void HeightFieldCPU::SetTerrainHeight(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	m_terrain_height.at(index) = value;
}
void HeightFieldCPU::SetVelocity_Z(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	m_velocity_z.at(index) = value;
}

/*
*=========================================================
*Set functions for a specified range of positions with value
*=========================================================
*
*/
void HeightFieldCPU::SetWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_water_height.at(index) = value;
		}
	}
}
void HeightFieldCPU::SetTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_terrain_height.at(index) = value;
		}
	}
}
void HeightFieldCPU::SetVelocity_Z(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_velocity_z.at(index) = value;
		}
	}
}

/*
*=========================================================
*Add functions for all positions
*=========================================================
*
*/
void HeightFieldCPU::AddWaterHeight(float value)
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_water_height.at(index) += value;
		}
	}
}

void HeightFieldCPU::AddTerrainHeight(float value)
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_terrain_height.at(index) += value;
		}
	}
}

void HeightFieldCPU::AddVelocity_Z(float value)
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_velocity_z.at(index) += value;
		}
	}
}

/*
*=========================================================
*Add functions for specific position
*=========================================================
*
*/
void HeightFieldCPU::AddWaterHeight(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	m_water_height.at(index) += value;
}
void HeightFieldCPU::AddTerrainHeight(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	m_terrain_height.at(index) += value;
}
void HeightFieldCPU::AddVelocity_Z(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	m_velocity_z.at(index) += value;
}
/*
*=========================================================
*Add functions for a range of positions
*=========================================================
*
*/
void HeightFieldCPU::AddWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_water_height.at(index) += value;
		}
	}
}
void HeightFieldCPU::AddTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_terrain_height.at(index) += value;
		}
	}
}
void HeightFieldCPU::AddVelocity_Z(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_velocity_z.at(index) += value;
		}
	}
}
/*
*=========================================================
*Subb functions for all positions
*=========================================================
*
*/
void HeightFieldCPU::SubWaterHeight(float value)
{
	AddWaterHeight(-value);
}
void HeightFieldCPU::SubTerrainHeight(float value)
{
	AddTerrainHeight(-value);
}
void HeightFieldCPU::SubVelocity_Z(float value)
{
	AddVelocity_Z(-value);
}
/*
*=========================================================
*Subb functions for a specific positions
*=========================================================
*
*/
void HeightFieldCPU::SubWaterHeight(float value, unsigned int x, unsigned int y)
{
	AddWaterHeight(-value, x, y);
}
void HeightFieldCPU::SubTerrainHeight(float value, unsigned int x, unsigned int y)
{
	AddTerrainHeight(-value, x, y);
}
void HeightFieldCPU::SubVelocity_Z(float value, unsigned int x, unsigned int y)
{
	AddVelocity_Z(-value, x, y);
}
/*
*=========================================================
*Subb functions for a range of positions
*=========================================================
*
*/
void HeightFieldCPU::SubWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	AddWaterHeight(-value, from_x, to_x, from_y, to_y);
}
void HeightFieldCPU::SubTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	AddTerrainHeight(-value, from_x, to_x, from_y, to_y);
}
void HeightFieldCPU::SubVelocity_Z(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	AddVelocity_Z(-value, from_x, to_x, from_y, to_y);
}



HeightFieldCPU::HeightFieldCPU(const unsigned int sizeX, const unsigned int sizeY) : m_sizeX(sizeX), m_sizeY(sizeY)
{
	m_terrain_height.resize(sizeX*sizeY);
	m_water_height.resize(sizeX*sizeY);
	temp.resize(sizeX*sizeY);
	m_velocity_z.resize(sizeX*sizeY);
}


HeightFieldCPU::~HeightFieldCPU()
{
}
