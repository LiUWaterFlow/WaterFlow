#include "shallowWater3.h"
#include <algorithm>
#include <iomanip>
#include <assert.h>
#include <math.h>

ShallowWater3::ShallowWater3(const unsigned int sizeX, const unsigned int sizeY) : m_sizeX(sizeX), m_sizeY(sizeY)
{
	m_grid.resize(sizeX*sizeY);
	m_temp.resize(sizeX*sizeY);
}
ShallowWater3::~ShallowWater3()
{
	m_grid.clear();
	m_temp.clear();
}

int ShallowWater3::run()
{
	/*Start by setting data*/
	AddTerrainHeight(10.0f);
	AddVelocity_X(0);
	//Add Water
	AddWaterHeight(1, 3, 5, 3, 5);

	/*Maybe print so it is as it should be*/
	PrintWaterHeight();
	//PrintTerrainHeight();
	PrintWaterHeightSum();
	for (unsigned int i = 0; i < 100000; i++) //maybe run the simulation 5 times
	{
		RunSimulation(0.05f);
		
		//if (i % 500 == 0)
		//{
			//Print velocity y for each iteration maybe
			//PrintWaterBool(i);
			//PrintWaterFillLevel(i);
			//PrintTerrainHeight(i);
			PrintWaterHeight(i);
			//PrintVelocity_X(i);
			//PrintVelocity_Y(i);
			PrintWaterHeightSum(i);
			//and pause so we see what happens
			Pause();
		//}
		//PrintWaterHeight(i);
		//PrintMomentum_X(i);
		//PrintMomentum_Y(i);
		//Pause();
	}
	/*then print again*/
	//PrintWaterHeight();
	//PrintWaterHeightSum();
	/*maybe a pause with a message so we know WHY we paused*/
	Pause("Last Pause before termination");

	return 1; //then end it here
}

void ShallowWater3::RunSimulation(const float timestep)
{
	for(unsigned int x = 1; x < m_sizeX - 1; x++)
	{
		for(unsigned int y = 1; y < m_sizeY - 1; y++)
		{
			gridPoint center = grid(x,y, m_grid);
			gridPoint north = grid(x, y - 1, m_grid);
			gridPoint west = grid(x - 1, y, m_grid);
			gridPoint south = grid(x, y + 1, m_grid);
			gridPoint east = grid(x + 1, y, m_grid);

			std::cout << "Before Fixing shores" << std::endl;
			Print

			fixShore(west, center, east);
			fixShore(north, center, south);
			std::cout << "After Fixing shores" << std::endl;
			PrintWaterHeightSum();

			gridPoint u_south = 0.5f * ( south + center) - timestep *(VerticalPotential(south) - VerticalPotential(center));
			gridPoint u_north = 0.5f * (north + center) - timestep * (VerticalPotential(center) - VerticalPotential(north));
			gridPoint u_west = 0.5f * (west + center) - timestep * (HorizontalPotential(center) - HorizontalPotential(west));
			gridPoint u_east = 0.5f * (east + center) - timestep * (HorizontalPotential(east) - HorizontalPotential(center));

			gridPoint u_center = center + timestep * SlopeForce(center, north, east, south, west) - 
							timestep* (HorizontalPotential(u_east) - HorizontalPotential(u_west)) -
							timestep * ( VerticalPotential(u_south) - VerticalPotential(u_north));

			std::cout << "After u_center" << std::endl;
			PrintWaterHeightSum();

			u_center.x = std::max(0.0f, u_center.x);
			
			grid(x, y, m_temp) = u_center;
		}
	}
	std::swap(m_temp, m_grid);
	//ResetTemp();
	//fixBoundry();
}

void ShallowWater3::fixShore(gridPoint& left, gridPoint& center, gridPoint& right)
{
	if(right.x < 0.0f || left.x < 0.0f || center.x < 0.0f)
	{
		center.x = center.x + left.x + right.x;
		center.x = std::max(0.0f,center.x);
		left.x = 0.0f;
		right.x = 0.0f;
	}

	float h = center.x;
	float h4 = h * h * h * h;
	
	float u = sqrt(2.0f) * h * center.y / (sqrt(h4 + std::max(h4, EPSILON)));
	float v = sqrt(2.0f) * h * center.z / (sqrt(h4 + std::max(h4, EPSILON)));
	
	center.y = u*h;
	center.z = v*h;
}

void ShallowWater3::fixBoundry()
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		grid(x, 0, m_grid).x = grid(x, 1, m_grid).x;
	}
	for (unsigned int y = 0; y < m_sizeY; y++)
	{
		grid(0, y, m_grid).x = -grid(1, y, m_grid).x;
	}
}

gridPoint ShallowWater3::SlopeForce(gridPoint center, gridPoint north, gridPoint east, gridPoint south, gridPoint west)
{
	float h = std::max(center.x, 0.0f);

	gridPoint H;
	H.x = 0.0f;
	H.y = -GRAV * h * (east.w - west.w);
	H.z = -GRAV * h * (south.w - north.w);
	H.w = 0.0f;
	return H;
}

gridPoint ShallowWater3::HorizontalPotential(gridPoint gp)
{
	float h = std::max(gp.x, 0.0f);
	float uh = gp.y;
	float vh = gp.z;
	
	float h4 = h * h * h * h;
	float u = sqrt(2.0f)* h * uh / (sqrt(h4 + std::max(h4, EPSILON)));
	
	gridPoint F;
	F.x = h * u;
	F.y = uh * u + GRAV*h*h;
	F.z = vh *u;
	F.w = 0.0f;
	return F;
}

gridPoint ShallowWater3::VerticalPotential(gridPoint gp)
{
	float h = std::max(gp.x,0.0f);
	float uh = gp.y;
	float vh = gp.z;
	
	float h4 = h * h * h * h;
	float v = sqrt(2.0f)* h * vh / (sqrt(h4 + std::max(h4, EPSILON)));
	
	gridPoint G;
	G.x = h * v;
	G.y = uh * v;
	G.z = vh * v + GRAV*h*h;
	G.w = 0.0f;
	return G;
}

gridPoint& ShallowWater3::grid(unsigned int x, unsigned int y, std::vector<gridPoint>& arr)
{
	return arr.at(x + m_sizeX*y);
}

void ShallowWater3::ResetTemp()
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			grid(x, y, m_temp).x = 0;
			grid(x, y, m_temp).y = 0;
			grid(x, y, m_temp).z = 0;
			grid(x, y, m_temp).w = 0;
		}
	}
}


/*functions for setting all values to a specific value*/
void ShallowWater3::SetWaterHeight(float value)
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			grid(x, y, m_grid).x = value;
		}
	}
}
void ShallowWater3::SetTerrainHeight(float value)
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			grid(x, y, m_grid).w = value;
		}
	}
}
void ShallowWater3::SetMomentum_X(float value)
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			grid(x, y, m_grid).y = value;
		}
	}
}
void ShallowWater3::SetMomentum_Y(float value)
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			grid(x, y, m_grid).z = value;
		}
	}
}
/*functions for setting a specific value at a specific place*/
void ShallowWater3::SetWaterHeight(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	grid(x, y, m_grid).x = value;
}
void ShallowWater3::SetTerrainHeight(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	grid(x, y, m_grid).w = value;
}
void ShallowWater3::SetMomentum_X(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	grid(x, y, m_grid).y = value;
}
void ShallowWater3::SetMomentum_Y(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	grid(x, y, m_grid).z = value;
}
/*functions for setting a specific value to a range*/
void ShallowWater3::SetWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			grid(x,y,m_grid).x = value;
		}
	}
}
void ShallowWater3::SetTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			grid(x, y, m_grid).w = value;
		}
	}
}
void ShallowWater3::SetMomentum_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			grid(x, y, m_grid).y = value;
		}
	}
}
void ShallowWater3::SetMomentum_Y(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			grid(x, y, m_grid).z = value;
		}
	}
}

/*functions for adding a value to all positions*/
void ShallowWater3::AddWaterHeight(float value)
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			grid(x,y,m_grid).x += value;
		}
	}
}
void ShallowWater3::AddTerrainHeight(float value)
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			grid(x, y, m_grid).w += value;
		}
	}
}
void ShallowWater3::AddMomentum_X(float value)
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			grid(x, y, m_grid).y += value;
		}
	}
}
void ShallowWater3::AddMomentum_Y(float value)
{
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			grid(x, y, m_grid).z += value;
		}
	}
}
/*functions for adding a value to specific position*/
void ShallowWater3::AddWaterHeight(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	grid(x,y,m_grid).x += value;
}
void ShallowWater3::AddTerrainHeight(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	grid(x, y, m_grid).w += value;
}
void ShallowWater3::AddMomentum_X(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	grid(x, y, m_grid).y += value;
}
void ShallowWater3::AddMomentum_Y(float value, unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	grid(x, y, m_grid).z += value;
}
/*functions for adding a value to a range of values*/
void ShallowWater3::AddWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			grid(x,y,m_grid).x += value;
		}
	}
}
void ShallowWater3::AddTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			grid(x, y, m_grid).w += value;
		}
	}
}
void ShallowWater3::AddMomentum_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			grid(x, y, m_grid).y += value;
		}
	}
}
void ShallowWater3::AddMomentum_Y(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			grid(x, y, m_grid).z += value;
		}
	}
}

/*functions for subbing a value to all positions*/
void ShallowWater3::SubWaterHeight(float value)
{
	AddWaterHeight(-value);
}
void ShallowWater3::SubTerrainHeight(float value)
{
	AddTerrainHeight(-value);
}
void ShallowWater3::SubMomentum_X(float value)
{
	AddMomentum_X(-value);
}
void ShallowWater3::SubMomentum_Y(float value)
{
	AddMomentum_Y(-value);
}
/*functions for subbing a value to specific position*/
void ShallowWater3::SubWaterHeight(float value, unsigned int x, unsigned int y)
{
	AddWaterHeight(-value, x, y);
}
void ShallowWater3::SubTerrainHeight(float value, unsigned int x, unsigned int y)
{
	AddTerrainHeight(-value, x, y);
}
void ShallowWater3::SubMomentum_X(float value, unsigned int x, unsigned int y)
{
	AddMomentum_X(-value, x, y);
}
void ShallowWater3::SubMomentum_Y(float value, unsigned int x, unsigned int y)
{
	AddMomentum_Y(-value, x, y);
}
/*functions for subbing a value to a range of values*/
void ShallowWater3::SubWaterHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	AddWaterHeight(-value, from_x, to_x, from_y, to_y);
}
void ShallowWater3::SubTerrainHeight(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	AddTerrainHeight(-value, from_x, to_x, from_y, to_y);
}
void ShallowWater3::SubMomentum_X(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	AddMomentum_X(-value, from_x, to_x, from_y, to_y);
}
void ShallowWater3::SubMomentum_Y(float value, unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	AddMomentum_Y(-value, from_x, to_x, from_y, to_y);
}

/*Functions for printing to the console. iter is default -1 if any other value you print that too*/
void ShallowWater3::PrintWaterHeight(int iteration)
{
	Print(TYPE::WATER, "WATER_HEIGHT", iteration);
}
void ShallowWater3::PrintTerrainHeight(int iteration)
{
	Print(TYPE::TERRAIN, "TERRAIN_HEIGHT", iteration);
}
void ShallowWater3::PrintMomentum_X(int iteration)
{
	Print(TYPE::MOM_X, "MOMENTUM_X", iteration);
}
void ShallowWater3::PrintMomentum_Y(int iteration)
{
	Print(TYPE::MOM_Y, "MOMENTUM_Y", iteration);
}
/*functions for printing a specific value at a specific position*/
void ShallowWater3::PrintWaterHeight(unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	PrintNumber(TYPE::WATER,grid(x,y,m_grid));
	std::cout << std::flush;
}
void ShallowWater3::PrintTerrainHeight(unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	PrintNumber(TYPE::TERRAIN, grid(x, y, m_grid));
	std::cout << std::flush;
}
void ShallowWater3::PrintMomentum_X(unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	PrintNumber(TYPE::MOM_X, grid(x, y, m_grid));
	std::cout << std::flush;
}
void ShallowWater3::PrintMomentum_Y(unsigned int x, unsigned int y)
{
	assert(x >= 0 && x <= m_sizeX && y >= 0 && y <= m_sizeY); //make sure we dont go outside array size
	PrintNumber(TYPE::MOM_Y, grid(x, y, m_grid));
	std::cout << std::flush;
}
/*functions to print values from a range*/
void ShallowWater3::PrintWaterHeight(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			PrintNumber(TYPE::WATER, grid(x, y, m_grid));
		}
	}
	std::cout << std::flush;
}
void ShallowWater3::PrintTerrainHeight(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			PrintNumber(TYPE::TERRAIN, grid(x, y, m_grid));
		}
	}
	std::cout << std::flush;
}
void ShallowWater3::PrintMomentum_X(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			PrintNumber(TYPE::MOM_X, grid(x, y, m_grid));
		}
	}
	std::cout << std::flush;
}
void ShallowWater3::PrintMomentum_Y(unsigned int from_x, unsigned int to_x, unsigned int from_y, unsigned int to_y)
{
	assert(from_x >= 0 && to_x <= m_sizeX && from_y >= 0 && to_y <= m_sizeY); //make sure we dont go outside array size
	for (unsigned int x = from_x; x < to_x; x++)
	{
		for (unsigned int y = from_y; y < to_y; y++)
		{
			PrintNumber(TYPE::MOM_Y, grid(x, y, m_grid));
		}
	}
	std::cout << std::flush;
}
/*Helper functions to pause and wait for input*/
void ShallowWater3::Pause() const
{
	std::getchar();
}
void ShallowWater3::Pause(std::string msg)
{
	std::cout << msg << std::endl;
	std::getchar();
}
void ShallowWater3::PrintNumber(TYPE type,gridPoint& gp)
{
	float value;
	switch (type)
	{
	case WATER:
		value = gp.x;
		break;
	case TERRAIN:
		value = gp.w;
		break;
	case MOM_X:
		value = gp.y;
		break;
	case MOM_Y:
		value = gp.z;
		break;
	default:
		break;
	}
	std::cout << std::fixed << std::setw(7) << std::setprecision(3) << value << " ";
}
void ShallowWater3::PrintHelper(std::string start_end, std::string msg, int iter)
{
	if (iter != -1) {
		std::cout << "========== " << start_end << " " << msg << " " << iter << " ==========\n\n";
	}
	else {
		std::cout << "========== " << start_end << " " << msg << " ==========\n\n";
	}
}
void ShallowWater3::Print(TYPE type, std::string msg, int iter)
{
	PrintHelper("BEGIN", msg, iter);
	for (unsigned int x = 0; x < m_sizeX; x++)
	{
		for (unsigned int y = 0; y < m_sizeY; y++)
		{
			PrintNumber(type, grid(x, y, m_grid));
		}
		std::cout << "\n";
	}
	PrintHelper("END", msg, iter);
	std::cout << std::flush;
}


float ShallowWater3::SumArray(TYPE type)
{
	float sum = 0.0f;
	for (unsigned int x = 1; x < m_sizeX - 1; x++)
	{
		for (unsigned int y = 1; y < m_sizeY - 1; y++)
		{
			const unsigned int index = x + y * m_sizeX;
			switch (type)
			{
			case WATER:
				sum += grid(x,y,m_grid).x;
				break;
			case TERRAIN:
				sum += grid(x, y, m_grid).w;
				break;
			case MOM_X:
				sum += grid(x, y, m_grid).y;
				break;
			case MOM_Y:
				sum += grid(x, y, m_grid).z;
				break;
			default:
				break;
			}
		}
	}
	return sum;
}

void ShallowWater3::PrintWaterHeightSum(int iter)
{
	SumOfArray = SumArray(TYPE::WATER);
	SumDifference = SumOfArray / OldSumArray - 1;
	OldSumArray = SumOfArray;

	std::cout << "Water Height Summation: " << SumOfArray;
	if (iter != -1)
	{
		std::cout << " Iteration: " << iter;
	}
	std::cout << " Percentage difference: " << SumDifference << " \n" << std::endl;

	//oldSumArray/
}