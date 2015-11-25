#include "ShallowWater2.h"
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <stdbool.h> //so bools can be passed with typename in printValue

int ShallowWater2::run()
{
	/*Start by setting data*/
	AddTerrainHeight(10.0f);
	AddVelocity_X(3);
	AddWaterHeight(10, 5, 5);
	/*Maybe print so it is as it should be*/
	PrintWaterHeight();
	for (unsigned int i = 0; i < 5; i++) //maybe run the simulation 5 times
	{
		RunSimulation(0.1f);
		/*Print velocity y for each iteration maybe*/
		PrintVelocity_Y(i);
		/*and pause so we see what happens*/
		Pause();
	}
	/*then print again*/
	PrintWaterHeight();
	/*maybe a pause with a message so we know WHY we paused*/
	Pause("Last Pause before termination");

	return 1; //then end it here
}

float ShallowWater2::bilinjearInterpolation(std::vector<float>& array, float point_x, float point_y)
{
	//picking out the nearby points
	const int X = (int)point_x;
	const int Y = (int)point_y;

	//picking out the distances to the point from nearby points
	const float s1 = point_x - X;
	const float s0 = 1.0f - s1;

	const float t1 = point_y - Y;
	const float t0 = 1.0f - t1;

	return s0*(t0*array[X + m_sizeX*Y] + t1*array[X + m_sizeX*(Y + 1)]) +
		s1*(t0*array[(X + 1) + m_sizeX*Y] + t1*array[(X + 1) + m_sizeX*(Y + 1)]);
	
}

//update the current height by looking backwards
void ShallowWater2::Advect(std::vector<float>& array,const float dt, VELTYPE type)
{
	for (unsigned int i = 1; i < m_sizeX - 1; i++)
	{
		for (unsigned int j = 1; j < m_sizeY - 1; j++)
		{
			const int index = i + j*m_sizeX;

			//only do following calculations if water is present
			if (m_fluid[index])
			{
				float u = 0; //speed x
				float v = 0; //speed y
				switch (type)
				{
				case ShallowWater2::VELTYPE::HEIGHT:
					u += (m_velocity_x[index] + m_velocity_x[index + 1])*0.5f;
					v += (m_velocity_y[index] + m_velocity_y[index + m_sizeX])*0.5f;
					break;
				case ShallowWater2::VELTYPE::X_VEL:
					u += m_velocity_x[index];
					v += (m_velocity_y[index] + m_velocity_y[index + 1] + m_velocity_y[index + m_sizeX] + m_velocity_y[index + m_sizeX + 1])*0.25f;
					break;
				case ShallowWater2::VELTYPE::Y_VEL:
					u += (m_velocity_x[index] + m_velocity_x[index + 1] + m_velocity_x[index + m_sizeX] + m_velocity_x[index + 1 + m_sizeX])*0.25f;
					v += m_velocity_y[index];
					break;
				default:
					exit(1); //should not happen
					break;
				}

				//backtracing
				float source_point_x = (float)i - u*dt; //divide by one step size
				float source_point_y = (float)j - u*dt;

				//making sure we dont go outside the bounds
				if (source_point_x < 0.0f){ source_point_x = 0.0f; }
				if (source_point_y < 0.0f){ source_point_y = 0.0f; }

				if (source_point_x > m_sizeX){ source_point_x = (float)m_sizeX - 1; }
				if (source_point_y > m_sizeY){ source_point_y = (float)m_sizeY - 1; }

				temp[index] = bilinjearInterpolation(array, source_point_x, source_point_y);
			}
		}
	}
	std::swap(temp, array);
}

void ShallowWater2::UpdateHeight(const float dt)
{
	for (unsigned int i = 0; i < m_sizeX - 1; i++)
	{
		for (unsigned int j = 0; j < m_sizeY - 1; j++)
		{
			const int index = i + j*m_sizeX;
			float delta_height = -0.5f*m_water_height[index] * (
				(m_velocity_x[index + 1] - m_velocity_x[index])
				+
				(m_velocity_y[index + m_sizeX] - m_velocity_y[index]));
			//update height
			m_water_height[index] += delta_height*dt;
		}
	}
}

void ShallowWater2::UpdateVelocity(const float dt, std::vector<float>& total_height) //this uses total height of water
{
	//update velocity in X
	for (unsigned int  i = 2; i < m_sizeX -1; i++)
	{
		for (unsigned int j = 1; j < m_sizeY - 1; j++)
		{
			const int index = i + j*m_sizeX;
			m_velocity_x[index] += GRAVITY* dt*(total_height[index] - total_height[index - 1]); //divide by cell size as well
		}

	}
	//update velocity in y
	for (unsigned int i = 1; i < m_sizeX - 1; i++)
	{
		for (unsigned int j = 2; j < m_sizeY - 1; j++)
		{
			const int index = i + j*m_sizeX;
			m_velocity_y[index] += GRAVITY*dt*(m_water_height[index] - m_water_height[index - m_sizeX]); //divide by cell size as well
		}
	}
}

void ShallowWater2::SetReflectBoundary()
{
	for (unsigned int i = 1; i < m_sizeX - 1; i++)
	{
		const int index1 = i + 0 * m_sizeX;
		const int index2 = i + (m_sizeY - 1)*m_sizeX; //this might be wrong
		m_water_height[index1] = m_water_height[index1 + m_sizeX];
		m_water_height[index2] = m_water_height[index2 - m_sizeX];
	}
	for (unsigned int j = 1; j < m_sizeY - 1; j++)
	{
		const int index1 = 0 + j*m_sizeX;
		const int index2 = (m_sizeX - 1) + j*m_sizeX;
		m_water_height[index1] = m_water_height[index1 + 1];
		m_water_height[index2] = m_water_height[index2 - 1];
	}
}

void ShallowWater2::AddTerrainHeight(std::vector<float>& array)
{
	for (unsigned int i = 0; i < m_sizeX; i++)
	{
		for (unsigned int j = 0; j < m_sizeY; j++)
		{
			const int index = i + j * m_sizeX;
			array[index] = m_water_height[index] + m_terrain_height[index];
		}
	}
}

void ShallowWater2::ComputeFluidFlags(std::vector<float>& ground_height, std::vector<float>& fluid_height)
{
	const float epsilon = 0.00001f;
	for (unsigned int i = 1; i < m_sizeX - 1; i++)
	{
		for (unsigned int j = 1; j < m_sizeY - 1; j++)
		{
			const int index = i + j*m_sizeX;
			const int left = i - 1 + j*m_sizeX;
			const int right = i + 1 + j*m_sizeX;
			const int above = i + (j + 1)*m_sizeX;
			const int below = i + (j - 1)*m_sizeX;

			float minHorizontalGround = std::min(ground_height[left], ground_height[right]);
			float minVerticalGround = std::min(ground_height[above], ground_height[below]);

			float maxHorizontalGround = std::max(ground_height[left], ground_height[right]);
			float maxVerticalGround = std::max(ground_height[above], ground_height[below]);

			float maxHorizontalFluid = std::max(fluid_height[left], fluid_height[right]);
			float maxVerticalFluid = std::max(fluid_height[above], fluid_height[below]);

			float h_min = std::min( minHorizontalGround, minVerticalGround);
			float h_max = std::max(maxHorizontalGround, maxVerticalGround);
			float fluid_max = std::max(maxHorizontalFluid, maxVerticalFluid);

			h_min = (ground_height[index] + h_min) / 2;
			h_max = (ground_height[index] + h_max) / 2 + epsilon; //to make sure that h_min != h_max
			fluid_max = (fluid_height[index] + fluid_max) / 2;

			if (m_water_height[index] <= h_min && fluid_max < epsilon) //fluid_max < epsilon can be seen as a theshhold for inflow
			{
				m_fluid[index] = false;
				m_fillLevel[index] = 0.0f;
			}
			else if (m_water_height[index] > h_max)
			{
				m_fluid[index] = true;
				m_fillLevel[index] = 1.0f;
			}
			else
			{
				m_fluid[index] = true;
				m_fillLevel[index] = (ground_height[index] - h_min) / (h_max - h_min);
			}
		}
	}
}

void ShallowWater2::RunSimulation(const float dt)
{
	ComputeFluidFlags(m_terrain_height, m_water_height);

	Advect(m_water_height,dt,VELTYPE::HEIGHT);
	Advect(m_velocity_x, dt, VELTYPE::X_VEL);
	Advect(m_velocity_y, dt, VELTYPE::Y_VEL);

	UpdateHeight(dt);
	AddTerrainHeight(temp); //make temp total height
	UpdateVelocity(dt, temp);

	SetReflectBoundary();
}

/*
*=========================================================
*Print function and helper print function
*=========================================================
*/
template <typename T>
void ShallowWater2::Print(std::vector<T> arr, std::string msg, int iter) const
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
template <typename T>
void ShallowWater2::PrintNumber(T& value) const
{
	std::cout << std::fixed << std::setw(7) << std::setprecision(3) << value << " ";
}
void ShallowWater2::PrintHelper(std::string start_end, std::string msg, int iter) const
{
	if (iter != -1) {
		std::cout << "========== " << start_end << " " <<  msg << " " << iter << " ==========\n\n";
	}
	else {
		std::cout << "========== " << start_end << " " << msg << " ==========\n\n";
	}
}
void ShallowWater2::Pause() const
{
	std::getchar();
}
void ShallowWater2::Pause(std::string msg) const
{
	std::cout << msg << std::endl;
	std::getchar();
}


/*
 *=========================================================
 *Print functions showing number of iterations
 *No iteration input gives no numbering
 *=========================================================
 */
void ShallowWater2::PrintWaterHeight(int iteration) const
{
	Print(m_water_height, "WATER_HEIGHT", iteration);
}
void ShallowWater2::PrintTerrainHeight(int iteration) const
{
	Print(m_terrain_height, "TERRAIN_HEIGHT", iteration);
}
void ShallowWater2::PrintVelocity_X(int iteration) const
{
	Print(m_velocity_x, "VELOCITY_X", iteration);
}
void ShallowWater2::PrintVelocity_Y(int iteration) const
{
	Print(m_velocity_y, "VELOCITY_Y", iteration);
}
void ShallowWater2::PrintWaterBool(int iteration) const
{
	Print(m_fluid, "WATER_BOOL", iteration);
}
void ShallowWater2::PrintWaterFillLevel(int iteration) const
{
	Print(m_fillLevel, "WATER_FILL_LEVEL", iteration);
}

/*
*=========================================================
*Print functions based on Index
*=========================================================
*Warning: Bool is strange, needed to move PrintNumber thingie over here
*/
void ShallowWater2::PrintWaterHeight(unsigned int x, unsigned int y) const
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	PrintNumber(m_water_height.at(index));
	std::cout << std::flush;
}
void ShallowWater2::PrintTerrainHeight(unsigned int x, unsigned int y) const
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	PrintNumber(m_terrain_height.at(index));
	std::cout << std::flush;
}
void ShallowWater2::PrintVelocity_X(unsigned int x, unsigned int y) const
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	PrintNumber(m_velocity_x.at(index));
	std::cout << std::flush;
}
void ShallowWater2::PrintVelocity_Y(unsigned int x, unsigned int y) const
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	PrintNumber(m_velocity_y.at(index));
	std::cout << std::flush;
}
void ShallowWater2::PrintWaterBool(unsigned int x, unsigned int y) const
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	std::cout << std::fixed << std::setprecision(3) << m_fluid.at(index) << " ";
	std::cout << std::flush;
}
void ShallowWater2::PrintWaterFillLevel(unsigned int x, unsigned int y) const
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	PrintNumber(m_fillLevel.at(index));
	std::cout << std::flush;
}

/*
*=========================================================
*Print functions based on RANGE
*=========================================================
*
*/
void ShallowWater2::PrintWaterHeight(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const
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
void ShallowWater2::PrintTerrainHeight(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const
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
void ShallowWater2::PrintVelocity_X(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			PrintNumber(m_velocity_x.at(index));
		}
	}
	std::cout << std::flush;
}
void ShallowWater2::PrintVelocity_Y(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			PrintNumber(m_velocity_y.at(index));
		}
	}
	std::cout << std::flush;
}
void ShallowWater2::PrintWaterBool(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			std::cout << std::fixed << std::setprecision(3) << m_fluid.at(index) << " ";
		}
	}
	std::cout << std::flush;
}
void ShallowWater2::PrintWaterFillLevel(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y) const
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			PrintNumber(m_fillLevel.at(index));
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
void ShallowWater2::SetWaterHeight(float value)
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
void ShallowWater2::SetTerrainHeight(float value)
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
void ShallowWater2::setVelocity_X(float value)
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_velocity_x.at(index) = value;
		}
	}
}
void ShallowWater2::SetVelocity_Y(float value)
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_velocity_y.at(index) = value;
		}
	}
}
/*
*=========================================================
*Set functions for specified position with value
*=========================================================
*
*/
void ShallowWater2::SetWaterHeight(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	m_water_height.at(index) = value;
}
void ShallowWater2::SetTerrainHeight(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	m_terrain_height.at(index) = value;
}
void ShallowWater2::setVelocity_X(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	m_velocity_x.at(index) = value;
}
void ShallowWater2::SetVelocity_Y(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	m_velocity_y.at(index) = value;
}

/*
*=========================================================
*Set functions for a specified range of positions with value
*=========================================================
*
*/
void ShallowWater2::SetWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
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
void ShallowWater2::SetTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
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
void ShallowWater2::setVelocity_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_velocity_x.at(index) = value;
		}
	}
}
void ShallowWater2::SetVelocity_Y(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_velocity_y.at(index) = value;
		}
	}
}

/*
*=========================================================
*Add functions for all positions
*=========================================================
*
*/
void ShallowWater2::AddWaterHeight(float value)
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
void ShallowWater2::AddTerrainHeight(float value)
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
void ShallowWater2::AddVelocity_X(float value)
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_velocity_x.at(index) += value;
		}
	}
}
void ShallowWater2::AddVelocity_Y(float value)
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_velocity_y.at(index) += value;
		}
	}
}
/*
*=========================================================
*Add functions for specific position
*=========================================================
*
*/
void ShallowWater2::AddWaterHeight(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	m_water_height.at(index) += value;
}
void ShallowWater2::AddTerrainHeight(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	m_terrain_height.at(index) += value;
}
void ShallowWater2::AddVelocity_X(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	m_velocity_x.at(index) += value;
}
void ShallowWater2::AddVelocity_Y(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	const unsigned int index = x + y*m_sizeX;
	m_velocity_y.at(index) += value;
}
/*
*=========================================================
*Add functions for a range of positions
*=========================================================
*
*/
void ShallowWater2::AddWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
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
void ShallowWater2::AddTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
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
void ShallowWater2::AddVelocity_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_velocity_x.at(index) += value;
		}
	}
}
void ShallowWater2::AddVelocity_Y(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			const unsigned int index = x + y*m_sizeX;
			m_velocity_y.at(index) += value;
		}
	}
}
/*
*=========================================================
*Subb functions for all positions
*=========================================================
*
*/
void ShallowWater2::SubWaterHeight(float value)
{
	AddWaterHeight(-value);
}
void ShallowWater2::SubTerrainHeight(float value)
{
	AddTerrainHeight(-value);
}
void ShallowWater2::SubVelocity_X(float value)
{
	AddVelocity_X(-value);
}
void ShallowWater2::SubVelocity_Y(float value)
{
	AddVelocity_Y(-value);
}
/*
*=========================================================
*Subb functions for a specific positions
*=========================================================
*
*/
void ShallowWater2::SubWaterHeight(float value, unsigned int x, unsigned int y)
{
	AddWaterHeight(-value, x, y);
}
void ShallowWater2::SubTerrainHeight(float value, unsigned int x, unsigned int y)
{
	AddTerrainHeight(-value, x, y);
}
void ShallowWater2::SubVelocity_X(float value, unsigned int x, unsigned int y)
{
	AddVelocity_X(-value, x, y);
}
void ShallowWater2::SubVelocity_Y(float value, unsigned int x, unsigned int y)
{
	AddVelocity_Y(-value, x, y);
}
/*
*=========================================================
*Subb functions for a range of positions
*=========================================================
*
*/
void ShallowWater2::SubWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	AddWaterHeight(-value, from_x, to_x, from_y, to_y);
}
void ShallowWater2::SubTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	AddTerrainHeight(-value, from_x, to_x, from_y, to_y);
}
void ShallowWater2::SubVelocity_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	AddVelocity_X(-value, from_x, to_x, from_y, to_y);
}
void ShallowWater2::SubVelocity_Y(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	AddVelocity_Y(-value, from_x, to_x, from_y, to_y);
}

/*
*=========================================================
*Constructor and Destructor
*=========================================================
*
*/
ShallowWater2::ShallowWater2(const unsigned int sizeX, const unsigned int sizeY) : m_sizeX(sizeX), m_sizeY(sizeY)
{
	m_water_height.resize(sizeX*sizeY);
	m_velocity_x.resize(sizeX*sizeY);
	m_velocity_y.resize(sizeX*sizeY);
	m_terrain_height.resize(sizeX*sizeY);
	m_fillLevel.resize(sizeX*sizeY);
	m_fluid.resize(sizeX*sizeY);
	temp.resize(sizeX*sizeY);
};

ShallowWater2::~ShallowWater2()
{
}