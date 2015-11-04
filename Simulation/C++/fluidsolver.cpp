#include "fluidsolver.h"

#include "iostream"

void FluidSolver::diffuse_one_velocity(float constantData, NeighbourVoxels& vox)
{
	//constantData =  a=dt*diff*N*N*N;
	vox.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity = 
		(vox.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity +
		constantData *
			(vox.voxels[CUBEPOS::CURRENT_MID_LEFT]->prev_velocity +
			vox.voxels[CUBEPOS::CURRENT_MID_RIGHT]->prev_velocity +
			vox.voxels[CUBEPOS::CURRENT_TOP_CENTER]->prev_velocity +
			vox.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->prev_velocity +
			vox.voxels[CUBEPOS::BACK_MID_CENTER]->prev_velocity +
			vox.voxels[CUBEPOS::FRONT_MID_CENTER]->prev_velocity)
		) / (1 + 6 * constantData);
};

void FluidSolver::diffuse_one_density(float constantData, NeighbourVoxels& vox)
{
	//constantData =  a=dt*diff*N*N*N;
	vox.voxels[CUBEPOS::CURRENT_MID_CENTER]->prev_density = 
		(vox.voxels[CUBEPOS::CURRENT_MID_CENTER]->density + 
		constantData * 
			(vox.voxels[CUBEPOS::CURRENT_MID_LEFT]->prev_density +
			vox.voxels[CUBEPOS::CURRENT_MID_RIGHT]->prev_density + 
			vox.voxels[CUBEPOS::CURRENT_TOP_CENTER]->prev_density +
			vox.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->prev_density + 
			vox.voxels[CUBEPOS::BACK_MID_CENTER]->prev_density + 
			vox.voxels[CUBEPOS::FRONT_MID_CENTER]->prev_density)
		) / (1 + 6 * constantData);
};

void FluidSolver::diffuse_velocity(float dt)
{
	float someconstant = dt;  // 
	NeighbourVoxels temp;
	//not including borders
	for (unsigned int k = 0; k < LIN_SOLVE; k++)
	{
		for (unsigned int x = 1; x < m_grid.XLength - 1; x++)
		{
			for (unsigned int y = 1; y < m_grid.YLength - 1; y++)
			{
				for (unsigned int z = 1; z < m_grid.ZLength - 1; z++)
				{
					temp = m_grid.getNeighbour(x, y, z);
					diffuse_one_velocity(someconstant, temp);
				}
			}
		}
		//set bounds for velocity
	}
};

void FluidSolver::diffuse_density(float dt)
{
	float someconstant = dt;  // 
	NeighbourVoxels temp;
	//not including borders
	for (unsigned int k = 0; k < LIN_SOLVE; k++)
	{
		for (unsigned int x = 1; x < m_grid.XLength - 1; x++)
		{
			for (unsigned int y = 1; y < m_grid.YLength - 1; y++)
			{
				for (unsigned int z = 1; z < m_grid.ZLength - 1; z++)
				{
					temp = m_grid.getNeighbour(x, y, z);
					diffuse_one_density(someconstant, temp);
				}
			}
		}
		//set bounds for density
	}
};

//Advect moves stuff around in the system
void FluidSolver::advect_velocity(float dt)
{
	float someconstant = dt;
	Voxel* temp;
	glm::ivec3 prev_gridPosition;
	glm::vec3 pointPosition;
	//not including borders
	for (unsigned int x = 1; x < m_grid.XLength - 1; x++)
	{
		for (unsigned int y = 1; y < m_grid.YLength - 1; y++)
		{
			for(unsigned int z = 1; z < m_grid.ZLength - 1; z++)
			{
				temp = m_grid.getVoxel(x, y, z);
				advect_core_function(someconstant, prev_gridPosition, glm::ivec3(x, y, z), pointPosition, temp->velocity);
				advect_one_velocity(someconstant, prev_gridPosition, pointPosition, temp);
			}
		}
	}	
};

void FluidSolver::advect_one_velocity(float constantData, glm::ivec3 prev_grid_position, glm::vec3 point_position, Voxel* currentVox)
{
	NeighbourVoxels origintemp = m_grid.getNeighbour(prev_grid_position.x, prev_grid_position.y, prev_grid_position.z);
	//pick out how far away from the voxel we stand in to the point
	float s1 = point_position.x - prev_grid_position.x;
	float s0 = 1 - s1;
	float t1 = point_position.y - prev_grid_position.y;
	float t0 = 1 - t1;
	float q1 = point_position.z - prev_grid_position.z;
	float q0 = 1 - q1;

	currentVox->velocity = 
		q0*(
			s0*(
				t0*origintemp.voxels[CUBEPOS::CURRENT_MID_CENTER]->prev_velocity
				+
				t1*origintemp.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->prev_velocity)
			+
			s1*(
				t0*origintemp.voxels[CUBEPOS::CURRENT_MID_RIGHT]->prev_velocity
				+
				t1*origintemp.voxels[CUBEPOS::CURRENT_BOTTOM_RIGHT]->prev_velocity))
		+
		q1*(
			s0*(
				t0*origintemp.voxels[CUBEPOS::FRONT_MID_CENTER]->prev_velocity
				+
				t1*origintemp.voxels[CUBEPOS::FRONT_BOTTOM_CENTER]->prev_velocity)
			+
			s1*(
				t0*origintemp.voxels[CUBEPOS::FRONT_MID_RIGHT]->prev_velocity
				+
				t1*origintemp.voxels[CUBEPOS::FRONT_BOTTOM_RIGHT]->prev_velocity));
};

void FluidSolver::advect_density(float dt)
{
	float someconstant = dt;
	Voxel* temp;
	glm::ivec3 prev_gridPosition;
	glm::vec3 pointPosition;
	//not including borders
	for (unsigned int x = 1; x < m_grid.XLength - 1; x++)
	{
		for (unsigned int y = 1; y < m_grid.YLength - 1; y++)
		{
			for (unsigned int z = 1; z < m_grid.ZLength - 1; z++)
			{
				temp = m_grid.getVoxel(x, y, z);
				advect_core_function(someconstant, prev_gridPosition, glm::vec3(x, y, z), pointPosition, temp->velocity);
				advect_one_density(someconstant, prev_gridPosition, pointPosition, temp);
			}
		}
	}
};

void FluidSolver::advect_core_function(float someconstant, glm::ivec3 &prev_gridPosition, glm::ivec3 gridPosition, glm::vec3 &pointPosition, const glm::vec3 &midVelocity)
{
	pointPosition = (glm::vec3)gridPosition - someconstant*midVelocity; //might be prev_velocity

	//if we are outsidde the inside voxels clamp it to the edges
	if (pointPosition.x < 0.5) {
		pointPosition.x = 0.5f;
	}
	else if (pointPosition.x >(m_grid.XLength - 1) + 0.5){
		pointPosition.x = (float)(m_grid.XLength - 1) + 0.5f;
	}
	if (pointPosition.y < 0.5) {
		pointPosition.y = 0.5f;
	}
	else if (pointPosition.y >(m_grid.YLength - 1) + 0.5) {
		pointPosition.y = (float)(m_grid.YLength - 1) + 0.5f;
	}
	if (pointPosition.z < 0.5) {
		pointPosition.z = 0.5f;
	}
	else if (pointPosition.z >(m_grid.ZLength - 1) + 0.5) {
		pointPosition.z = (float)(m_grid.ZLength - 1) + 0.5f;
	}
	//finding the previous position on the grid by clamping it to integers
	prev_gridPosition = glm::floor(pointPosition);
};

void FluidSolver::advect_one_density(float constantData, glm::ivec3 prev_grid_position, glm::vec3 point_position, Voxel* currentVox)
{
	NeighbourVoxels origintemp = m_grid.getNeighbour(prev_grid_position.x, prev_grid_position.y, prev_grid_position.z);
	//pick out how far away from the voxel we stand in to the point
	float s1 = point_position.x - prev_grid_position.x;
	float s0 = 1 - s1;
	float t1 = point_position.y - prev_grid_position.y;
	float t0 = 1 - t1;
	float q1 = point_position.z - prev_grid_position.z;
	float q0 = 1 - q1;

	currentVox->density =
		q0*(
			s0*(
				t0*origintemp.voxels[CUBEPOS::CURRENT_MID_CENTER]->prev_density
				+
				t1*origintemp.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->prev_density)
			+
			s1*(
				t0*origintemp.voxels[CUBEPOS::CURRENT_MID_RIGHT]->prev_density
				+
				t1*origintemp.voxels[CUBEPOS::CURRENT_BOTTOM_RIGHT]->prev_density))
		+
		q1*(
			s0*(
				t0*origintemp.voxels[CUBEPOS::FRONT_MID_CENTER]->prev_density
				+
				t1*origintemp.voxels[CUBEPOS::FRONT_BOTTOM_CENTER]->prev_density)
			+
			s1*(
				t0*origintemp.voxels[CUBEPOS::FRONT_MID_RIGHT]->prev_density
			+
				t1*origintemp.voxels[CUBEPOS::FRONT_BOTTOM_RIGHT]->prev_density));
};

void FluidSolver::project_velocity(float dt)
{
	NeighbourVoxels temp;
	for (unsigned int z = 0; z < m_grid.ZLength; z++)
	{
		for (unsigned int y = 0; y < m_grid.YLength; y++)
		{
			for (unsigned int x = 0; x < m_grid.XLength; x++)
			{
				temp = m_grid.getNeighbour(x, y, z);
				temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->divergence = -0.5f*(
					temp.voxels[CUBEPOS::CURRENT_MID_LEFT]->velocity.x - temp.voxels[CUBEPOS::CURRENT_MID_RIGHT]->velocity.x
					+ temp.voxels[CUBEPOS::CURRENT_TOP_CENTER]->velocity.y - temp.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->velocity.y
					+ temp.voxels[CUBEPOS::FRONT_MID_CENTER]->velocity.z - temp.voxels[CUBEPOS::BACK_MID_CENTER]->velocity.z); /* /N */
				temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->preassure = 0.0f;
			}
		}
	}
	//force boundries for divergence
	//force boundries for preassure

	for (unsigned int k= 0; k < LIN_SOLVE; k++)
	{
		for (unsigned int z = 0; z < m_grid.ZLength; z++)
		{
			for (unsigned int y = 0; y < m_grid.YLength; y++)
			{
				for (unsigned int x = 0; x < m_grid.XLength; x++)
				{
					temp = m_grid.getNeighbour(x, y, z);
					temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->preassure =
						(
						temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->divergence
						+ temp.voxels[CUBEPOS::CURRENT_MID_LEFT]->preassure + temp.voxels[CUBEPOS::CURRENT_MID_RIGHT]->preassure
						+ temp.voxels[CUBEPOS::CURRENT_TOP_CENTER]->preassure + temp.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->preassure
						+ temp.voxels[CUBEPOS::FRONT_MID_CENTER]->preassure + temp.voxels[CUBEPOS::BACK_MID_CENTER]->preassure
						) / 6;
				}
			}
		}
		//force boundries for preassure
	}

	for (unsigned int z = 0; z < m_grid.ZLength; z++)
	{
		for (unsigned int y = 0; y < m_grid.YLength; y++)
		{
			for (unsigned int x = 0; x < m_grid.XLength; x++)
			{
				temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity.x -= 0.5f*(temp.voxels[CUBEPOS::CURRENT_MID_LEFT]->preassure - temp.voxels[CUBEPOS::CURRENT_MID_RIGHT]->preassure); /* /N */
				temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity.y -= 0.5f*(temp.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->preassure - temp.voxels[CUBEPOS::CURRENT_TOP_CENTER]->preassure); /* /N */
				temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity.z -= 0.5f*(temp.voxels[CUBEPOS::BACK_MID_CENTER]->preassure - temp.voxels[CUBEPOS::FRONT_MID_CENTER]->preassure); /* /N */
			}
		}
	}
	//force boundries for velocity
};

void FluidSolver::force_boundries_velocity()
{
	std::vector<Voxel*> top = m_grid.getBorderTop();
	std::vector<Voxel*> bottom = m_grid.getBorderBottom();
	std::vector<Voxel*> left = m_grid.getBorderLeft();
	std::vector<Voxel*> right = m_grid.getBorderRight();
	std::vector<Voxel*> front = m_grid.getBorderFront();
	std::vector<Voxel*> back = m_grid.getBorderBack();

	for each (Voxel* it in top)
	{
		NeighbourVoxels temp = m_grid.getNeighbour(it);
		temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity.y = -temp.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->velocity.y;
	}
	for each (Voxel* it in bottom)
	{
		NeighbourVoxels temp = m_grid.getNeighbour(it);
		temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity.y = -temp.voxels[CUBEPOS::CURRENT_TOP_CENTER]->velocity.y;
	}
	for each (Voxel* it in left)
	{
		NeighbourVoxels temp = m_grid.getNeighbour(it);
		temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity.x = -temp.voxels[CUBEPOS::CURRENT_MID_RIGHT]->velocity.x;
	}
	for each (Voxel* it in right)
	{
		NeighbourVoxels temp = m_grid.getNeighbour(it);
		temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity.x = -temp.voxels[CUBEPOS::CURRENT_MID_LEFT]->velocity.x;
	}
	for each (Voxel* it in front)
	{
		NeighbourVoxels temp = m_grid.getNeighbour(it);
		temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity.z = -temp.voxels[CUBEPOS::BACK_MID_CENTER]->velocity.z;
	}
	for each (Voxel* it in back)
	{
		NeighbourVoxels temp = m_grid.getNeighbour(it);
		temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity.z = -temp.voxels[CUBEPOS::FRONT_MID_CENTER]->velocity.z;
	}

	//then do corners properly Dunno how to do it at the moment with next grid implimentation
	//Probably just check the corners....Just 8 special cases
}

void FluidSolver::force_boundries_density()
{
	std::vector<Voxel*> top = m_grid.getBorderTop();
	std::vector<Voxel*> bottom = m_grid.getBorderBottom();
	std::vector<Voxel*> left = m_grid.getBorderLeft();
	std::vector<Voxel*> right = m_grid.getBorderRight();
	std::vector<Voxel*> front = m_grid.getBorderFront();
	std::vector<Voxel*> back = m_grid.getBorderBack();

	for each (Voxel* it in top)
	{
		NeighbourVoxels temp = m_grid.getNeighbour(it);
		temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->density = temp.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->density;
	}
	for each (Voxel* it in bottom)
	{
		NeighbourVoxels temp = m_grid.getNeighbour(it);
		temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->density = temp.voxels[CUBEPOS::CURRENT_TOP_CENTER]->density;
	}
	for each (Voxel* it in left)
	{
		NeighbourVoxels temp = m_grid.getNeighbour(it);
		temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->density = temp.voxels[CUBEPOS::CURRENT_MID_RIGHT]->density;
	}
	for each (Voxel* it in right)
	{
		NeighbourVoxels temp = m_grid.getNeighbour(it);
		temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->density = temp.voxels[CUBEPOS::CURRENT_MID_LEFT]->density;
	}
	for each (Voxel* it in front)
	{
		NeighbourVoxels temp = m_grid.getNeighbour(it);
		temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->density = temp.voxels[CUBEPOS::BACK_MID_CENTER]->density;
	}
	for each (Voxel* it in back)
	{
		NeighbourVoxels temp = m_grid.getNeighbour(it);
		temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->density = temp.voxels[CUBEPOS::FRONT_MID_CENTER]->density;
	}

	//then do corners properly Dunno how to do it at the moment with next grid implimentation
	//Probably just check the corners....Just 8 special cases
}

void FluidSolver::force_boundries_preassure()
{
	std::vector<Voxel*> top = m_grid.getBorderTop();
	std::vector<Voxel*> bottom = m_grid.getBorderBottom();
	std::vector<Voxel*> left = m_grid.getBorderLeft();
	std::vector<Voxel*> right = m_grid.getBorderRight();
	std::vector<Voxel*> front = m_grid.getBorderFront();
	std::vector<Voxel*> back = m_grid.getBorderBack();

	for each (Voxel* it in top)
	{
		NeighbourVoxels temp = m_grid.getNeighbour(it);
		temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->preassure = temp.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->preassure;
	}
	for each (Voxel* it in bottom)
	{
		NeighbourVoxels temp = m_grid.getNeighbour(it);
		temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->preassure = temp.voxels[CUBEPOS::CURRENT_TOP_CENTER]->preassure;
	}
	for each (Voxel* it in left)
	{
		NeighbourVoxels temp = m_grid.getNeighbour(it);
		temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->preassure = temp.voxels[CUBEPOS::CURRENT_MID_RIGHT]->preassure;
	}
	for each (Voxel* it in right)
	{
		NeighbourVoxels temp = m_grid.getNeighbour(it);
		temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->preassure = temp.voxels[CUBEPOS::CURRENT_MID_LEFT]->preassure;
	}
	for each (Voxel* it in front)
	{
		NeighbourVoxels temp = m_grid.getNeighbour(it);
		temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->preassure = temp.voxels[CUBEPOS::BACK_MID_CENTER]->preassure;
	}
	for each (Voxel* it in back)
	{
		NeighbourVoxels temp = m_grid.getNeighbour(it);
		temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->preassure = temp.voxels[CUBEPOS::FRONT_MID_CENTER]->preassure;
	}

	//then do corners properly Dunno how to do it at the moment with next grid implimentation
	//Probably just check the corners....Just 8 special cases
}