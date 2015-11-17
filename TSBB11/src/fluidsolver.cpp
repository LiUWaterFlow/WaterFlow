#include "fluidsolver.h"

#include "iostream"

#define BEGIN_PER_CELL for (unsigned int z = 1; z < m_grid->getZMaxSize() - 1; z++){ \
			for (unsigned int y = 1; y < m_grid->getYMaxSize() - 1; y++) { \
				for (unsigned int x = 1; x < m_grid->getXMaxSize() - 1; x++){
#define END_PER_CELL }}}


/*Linjearly solves (with the help of looping through all points and forcing boundries) the probelm Ax = b
 *Here A is c_left, c_right, c_above, c_below, c_near, c_far, That is the components we can reach from each voxel
 *b in this case is the previous value we have avaiable.
 *x is the current value at the center
 */
template <typename T>
void FluidSolver::linjear_solve_helper(float constantData, T& current_center, const T& prev_center, const T& c_left, const T& c_right, const T& c_above, const T& c_below, const T& c_near, const T& c_far)
{
	current_center = (prev_center + constantData * (c_left + c_right + c_above + c_below + c_near + c_far)) / (1 + 6 * constantData);
}

template <typename T>
void FluidSolver::advect_helper(glm::vec3 point_position, glm::vec3 prev_grid_position,
	T& searched, const T& prev_c_mid_center, const T& prev_near_mid_center, const T& prev_c_top_center,
	const T& prev_near_top_center, const T& prev_c_mid_right, const T& prev_near_mid_right, const T& prev_c_top_right,
	const T& prev_near_top_right)
{
float x1 = point_position.x - prev_grid_position.x;
float x0 = 1 - x1;
float y1 = point_position.y - prev_grid_position.y;
float y0 = 1 - y1;
float z1 = point_position.z - prev_grid_position.z;
float z0 = 1 - z1;

searched =
x0*(
	y0*( z0*prev_c_mid_center + z1*prev_near_mid_center )
	+
	y1*( z0*prev_c_top_center + z1*prev_near_top_center )
	
	)
+
x1*(
	y0*( z0*prev_c_mid_right + z1*prev_near_mid_right )
	+
	y1*( z0*prev_c_top_right + z1*prev_near_top_right )
	);
}


void FluidSolver::dens_step(float dt)
{
	diffuse_density(dt);
	advect_density(dt);
}

void FluidSolver::velocity_step(float dt)
{
	advect_velocity(dt);
	diffuse_velocity(dt);
	project_velocity(dt);
}

void FluidSolver::diffuse_one_velocity(float constantData, NeighbourVoxels& vox)
{
	//constantData = dt*diff*N*N*N;
	linjear_solve_helper(constantData, vox.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity,
		vox.voxels[CUBEPOS::CURRENT_MID_CENTER]->prev_velocity,
		vox.voxels[CUBEPOS::CURRENT_MID_LEFT]->prev_velocity,
		vox.voxels[CUBEPOS::CURRENT_MID_RIGHT]->prev_velocity,
		vox.voxels[CUBEPOS::CURRENT_TOP_CENTER]->prev_velocity,
		vox.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->prev_velocity,
		vox.voxels[CUBEPOS::FAR_MID_CENTER]->prev_velocity,
		vox.voxels[CUBEPOS::NEAR_MID_CENTER]->prev_velocity);
}

void FluidSolver::diffuse_one_density(float constantData, NeighbourVoxels& vox)
{
	//constantData = dt*diff*N*N*N;
	linjear_solve_helper(constantData, vox.voxels[CUBEPOS::CURRENT_MID_CENTER]->density,
		vox.voxels[CUBEPOS::CURRENT_MID_CENTER]->prev_density,
		vox.voxels[CUBEPOS::CURRENT_MID_LEFT]->prev_density,
		vox.voxels[CUBEPOS::CURRENT_MID_RIGHT]->prev_density,
		vox.voxels[CUBEPOS::CURRENT_TOP_CENTER]->prev_density,
		vox.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->prev_density,
		vox.voxels[CUBEPOS::FAR_MID_CENTER]->prev_density,
		vox.voxels[CUBEPOS::NEAR_MID_CENTER]->prev_density);

}

void FluidSolver::diffuse_velocity(float dt)
{
	float N = m_grid->getVoxelSize();
	float someconstant = dt*diffuse*N*N*N; 
	NeighbourVoxels temp;
	//not including borders
	for (unsigned int k = 0; k < LIN_SOLVE; k++)
	{
		BEGIN_PER_CELL
			temp = m_grid->getNeighbour(x, y, z);
			diffuse_one_velocity(someconstant, temp);
		END_PER_CELL
		//set bounds for velocity
		force_boundries_velocity();
	} //end of Lin solver
}

void FluidSolver::diffuse_density(float dt)
{
	float N = m_grid->getVoxelSize();
	float someconstant = dt*diffuse*N*N*N;
	NeighbourVoxels temp;
	//not including borders
	for (unsigned int k = 0; k < LIN_SOLVE; k++)
	{
		BEGIN_PER_CELL
			temp = m_grid->getNeighbour(x, y, z);
			diffuse_one_density(someconstant, temp);
		END_PER_CELL
		//set bounds for density
		force_boundries_density();
	} //end of lin solver
}

//Advect moves stuff around in the system
void FluidSolver::advect_velocity(float dt)
{
	float N = m_grid->getVoxelSize();
	float someconstant = dt*diffuse*N*N*N;
	voxel* temp;
	glm::ivec3 prev_gridPosition;
	glm::vec3 pointPosition;
	//not including borders
	BEGIN_PER_CELL
		temp = m_grid->getVoxel(x, y, z);
		advect_core_function(someconstant, prev_gridPosition, glm::ivec3(x, y, z), pointPosition, temp->velocity);
		advect_one_velocity(someconstant, prev_gridPosition, pointPosition, temp);
	END_PER_CELL	
}

void FluidSolver::advect_density(float dt)
{
	float someconstant = dt*m_grid->getVoxelSize();
	voxel* temp;
	glm::ivec3 prev_gridPosition;
	glm::vec3 pointPosition;
	//not including borders
	BEGIN_PER_CELL
		temp = m_grid->getVoxel(x, y, z);
		advect_core_function(someconstant, prev_gridPosition, glm::vec3(x, y, z), pointPosition, temp->velocity);
		advect_one_density(someconstant, prev_gridPosition, pointPosition, temp);
	END_PER_CELL
}

void FluidSolver::advect_core_function(float someconstant, glm::ivec3 &prev_gridPosition, glm::ivec3 gridPosition, glm::vec3 &pointPosition, const glm::vec3 &midVelocity)
{
	pointPosition = (glm::vec3)gridPosition - someconstant*midVelocity; //might be prev_velocity

	unsigned int maxX = m_grid->getXMaxSize();
	unsigned int maxY = m_grid->getYMaxSize();
	unsigned int maxZ = m_grid->getZMaxSize();

	//if we are outsidde the inside voxels clamp it to the edges
	if (pointPosition.x < 0.5) {
		pointPosition.x = 0.5f;
	}
	else if (pointPosition.x >(maxX - 1) + 0.5){
		pointPosition.x = (float)(maxX - 1) + 0.5f;
	}
	if (pointPosition.y < 0.5) {
		pointPosition.y = 0.5f;
	}
	else if (pointPosition.y >(maxY - 1) + 0.5) {
		pointPosition.y = (float)(maxY - 1) + 0.5f;
	}
	if (pointPosition.z < 0.5) {
		pointPosition.z = 0.5f;
	}
	else if (pointPosition.z >(maxZ - 1) + 0.5) {
		pointPosition.z = (float)(maxZ - 1) + 0.5f;
	}
	//finding the previous position on the grid by clamping it to integers
	prev_gridPosition = glm::floor(pointPosition);
}

void FluidSolver::advect_one_velocity(float constantData, glm::ivec3 prev_grid_position, glm::vec3 point_position, voxel* currentVox)
{
	NeighbourVoxels origintemp = m_grid->getNeighbour(prev_grid_position.x, prev_grid_position.y, prev_grid_position.z);
	//pick out how far away from the voxel we stand in to the point
	advect_helper(point_position, prev_grid_position, currentVox->velocity,
		origintemp.voxels[CUBEPOS::CURRENT_MID_CENTER]->prev_velocity,
		origintemp.voxels[CUBEPOS::NEAR_MID_CENTER]->prev_velocity,
		origintemp.voxels[CUBEPOS::CURRENT_TOP_CENTER]->prev_velocity,
		origintemp.voxels[CUBEPOS::NEAR_TOP_CENTER]->prev_velocity,
		origintemp.voxels[CUBEPOS::CURRENT_MID_RIGHT]->prev_velocity,
		origintemp.voxels[CUBEPOS::NEAR_MID_RIGHT]->prev_velocity,
		origintemp.voxels[CUBEPOS::CURRENT_TOP_RIGHT]->prev_velocity,
		origintemp.voxels[CUBEPOS::NEAR_TOP_RIGHT]->prev_velocity
		);
}

void FluidSolver::advect_one_density(float constantData, glm::ivec3 prev_grid_position, glm::vec3 point_position, voxel* currentVox)
{
	NeighbourVoxels origintemp = m_grid->getNeighbour(prev_grid_position.x, prev_grid_position.y, prev_grid_position.z);
	//pick out how far away from the voxel we stand in to the point
	advect_helper(point_position, prev_grid_position, currentVox->density,
		origintemp.voxels[CUBEPOS::CURRENT_MID_CENTER]->prev_density,
		origintemp.voxels[CUBEPOS::NEAR_MID_CENTER]->prev_density,
		origintemp.voxels[CUBEPOS::CURRENT_TOP_CENTER]->prev_density,
		origintemp.voxels[CUBEPOS::NEAR_TOP_CENTER]->prev_density,
		origintemp.voxels[CUBEPOS::CURRENT_MID_RIGHT]->prev_density,
		origintemp.voxels[CUBEPOS::NEAR_MID_RIGHT]->prev_density,
		origintemp.voxels[CUBEPOS::CURRENT_TOP_RIGHT]->prev_density,
		origintemp.voxels[CUBEPOS::NEAR_TOP_RIGHT]->prev_density
		);
}

void FluidSolver::project_velocity(float dt)
{
	NeighbourVoxels temp;
	float size_of_voxel = 1 / m_grid->getVoxelSize();
	BEGIN_PER_CELL
		temp = m_grid->getNeighbour(x, y, z);
	temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->divergence = -0.5f*(
		temp.voxels[CUBEPOS::CURRENT_MID_LEFT]->velocity.x - temp.voxels[CUBEPOS::CURRENT_MID_RIGHT]->velocity.x
		+ temp.voxels[CUBEPOS::CURRENT_TOP_CENTER]->velocity.y - temp.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->velocity.y
		+ temp.voxels[CUBEPOS::NEAR_MID_CENTER]->velocity.z - temp.voxels[CUBEPOS::FAR_MID_CENTER]->velocity.z) * size_of_voxel;
	
	temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->preassure = 0.0f;
	END_PER_CELL
	//force boundries for divergence
	force_boundries_divergence();
	//force boundries for preassure
	force_boundries_preassure();
	for (unsigned int k= 0; k < LIN_SOLVE; k++)
	{
		BEGIN_PER_CELL
			temp = m_grid->getNeighbour(x, y, z);
			temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->preassure =
				(
				temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->divergence
				+ temp.voxels[CUBEPOS::CURRENT_MID_LEFT]->preassure + temp.voxels[CUBEPOS::CURRENT_MID_RIGHT]->preassure
				+ temp.voxels[CUBEPOS::CURRENT_TOP_CENTER]->preassure + temp.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->preassure
				+ temp.voxels[CUBEPOS::NEAR_MID_CENTER]->preassure + temp.voxels[CUBEPOS::FAR_MID_CENTER]->preassure
				) / 6;
		END_PER_CELL
		//force boundries for preassure
		force_boundries_preassure();
	}

	BEGIN_PER_CELL
		temp = m_grid->getNeighbour(x, y, z);
	temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity.x -= 0.5f*(
		temp.voxels[CUBEPOS::CURRENT_MID_LEFT]->preassure - temp.voxels[CUBEPOS::CURRENT_MID_RIGHT]->preassure) * size_of_voxel;
	
	temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity.y -= 0.5f*(
		temp.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->preassure - temp.voxels[CUBEPOS::CURRENT_TOP_CENTER]->preassure) * size_of_voxel;
	
	temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity.z -= 0.5f*(
		temp.voxels[CUBEPOS::FAR_MID_CENTER]->preassure - temp.voxels[CUBEPOS::NEAR_MID_CENTER]->preassure) * size_of_voxel;
	END_PER_CELL
	//force boundries for velocity
	force_boundries_velocity();
}

//need to check how we define max sizes
void FluidSolver::force_boundries_velocity()
{
	unsigned int maxX = m_grid->getXMaxSize();
	unsigned int maxY = m_grid->getYMaxSize();
	unsigned int maxZ = m_grid->getZMaxSize();
	for (unsigned int i = 0; i <= maxX; i++)
	{
		for (unsigned int j = 0; j <= maxY; j++)
		{
			//near
			m_grid->getGuaranteedVoxel(i, j, 0)->velocity = -m_grid->getGuaranteedVoxel(i, j, 1)->velocity;
			//far
			m_grid->getGuaranteedVoxel(i, j, maxZ)->velocity = -m_grid->getGuaranteedVoxel(i, j, maxZ -1)->velocity;
		}
	}
	for (unsigned int i = 0; i <= maxX; i++)
	{
		for (unsigned int j = 0; j < maxZ; j++)
		{
			//bottom
			m_grid->getGuaranteedVoxel(i, 0, j)->velocity = -m_grid->getGuaranteedVoxel(i, 1, j)->velocity;
			//top
			m_grid->getGuaranteedVoxel(i, maxY, j)->velocity = -m_grid->getGuaranteedVoxel(i, maxY - 1, j)->velocity;
		}
	}
	for (unsigned int i = 0; i < maxY; i++)
	{
		for (unsigned int j = 0; j < maxZ; j++)
		{
			//left
			m_grid->getGuaranteedVoxel(0, i, j)->velocity = -m_grid->getGuaranteedVoxel(1, i, j)->velocity;
			//right
			m_grid->getGuaranteedVoxel(maxX, i, j)->velocity = -m_grid->getGuaranteedVoxel(maxX - 1, i, j)->velocity;
		}
	}
	//left bottom near
	m_grid->getGuaranteedVoxel(0, 0, 0)->velocity = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(1, 0, 0)->velocity
		+ m_grid->getGuaranteedVoxel(0, 1, 0)->velocity + m_grid->getGuaranteedVoxel(0, 0, 1)->velocity);

	//top-left-near
	m_grid->getGuaranteedVoxel(0,maxY + 1,0)->velocity = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(1, maxY + 1, 0)->velocity
		+ m_grid->getGuaranteedVoxel(0, maxY, 0)->velocity + m_grid->getGuaranteedVoxel(0, maxY + 1, 1)->velocity);
	//bottom-right-near
	m_grid->getGuaranteedVoxel(maxX+1,0,0)->velocity = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(maxX,0,0)->velocity
		+ m_grid->getGuaranteedVoxel(maxX+1,1,0)->velocity + m_grid->getGuaranteedVoxel(maxX+1,0,1)->velocity);
	//right-top-near
	m_grid->getGuaranteedVoxel(maxX+1,maxY+1,0)->velocity = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(maxX,maxY+1,0)->velocity
		+ m_grid->getGuaranteedVoxel(maxX+1,maxY,0)->velocity + m_grid->getGuaranteedVoxel(maxX+1,maxY+1,1)->velocity);
	//left-bottom-far
	m_grid->getGuaranteedVoxel(0,0,maxZ+1)->velocity = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(1,0,maxZ+1)->velocity
		+ m_grid->getGuaranteedVoxel(0,1,maxZ+1)->velocity + m_grid->getGuaranteedVoxel(0,0,maxZ)->velocity);
	//left-top-far
	m_grid->getGuaranteedVoxel(0,maxY+1,maxZ+1)->velocity = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(1,maxY+1,maxZ+1)->velocity
		+ m_grid->getGuaranteedVoxel(0,maxY,maxZ+1)->velocity + m_grid->getGuaranteedVoxel(0,maxY+1,maxZ)->velocity);
	//right-bottom-far
	m_grid->getGuaranteedVoxel(maxX+1,0,maxZ+1)->velocity = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(maxX,0,maxZ+1)->velocity
		+ m_grid->getGuaranteedVoxel(maxX+1,1,maxZ+1)->velocity + m_grid->getGuaranteedVoxel(maxX+1,0,maxZ)->velocity);
	//right-top-far
	m_grid->getGuaranteedVoxel(maxX+1,maxY+1,maxZ+1)->velocity = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(maxX,maxY+1,maxZ+1)->velocity
		+ m_grid->getGuaranteedVoxel(maxX+1,maxY,maxZ+1)->velocity + m_grid->getGuaranteedVoxel(maxX+1,maxY+1,maxZ)->velocity);
}

void FluidSolver::force_boundries_density()
{
	unsigned int maxX = m_grid->getXMaxSize();
	unsigned int maxY = m_grid->getYMaxSize();
	unsigned int maxZ = m_grid->getZMaxSize();
	for (unsigned int i = 0; i <= maxX; i++)
	{
		for (unsigned int j = 0; j <= maxY; j++)
		{
			//near
			m_grid->getGuaranteedVoxel(i, j, 0)->density = m_grid->getGuaranteedVoxel(i, j, 1)->density;
			//far
			m_grid->getGuaranteedVoxel(i, j, maxZ)->density = m_grid->getGuaranteedVoxel(i, j, maxZ - 1)->density;
		}
	}
	for (unsigned int i = 0; i <= maxX; i++)
	{
		for (unsigned int j = 0; j < maxZ; j++)
		{
			//bottom
			m_grid->getGuaranteedVoxel(i, 0, j)->density = m_grid->getGuaranteedVoxel(i, 1, j)->density;
			//top
			m_grid->getGuaranteedVoxel(i, maxY, j)->density = m_grid->getGuaranteedVoxel(i, maxY - 1, j)->density;
		}
	}
	for (unsigned int i = 0; i < maxY; i++)
	{
		for (unsigned int j = 0; j < maxZ; j++)
		{
			//left
			m_grid->getGuaranteedVoxel(0, i, j)->density = m_grid->getGuaranteedVoxel(1, i, j)->density;
			//right
			m_grid->getGuaranteedVoxel(maxX, i, j)->density = m_grid->getGuaranteedVoxel(maxX - 1, i, j)->density;
		}
	}
	//left bottom near
	m_grid->getGuaranteedVoxel(0, 0, 0)->density = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(1, 0, 0)->density
		+ m_grid->getGuaranteedVoxel(0, 1, 0)->density + m_grid->getGuaranteedVoxel(0, 0, 1)->density);

	//top-left-near
	m_grid->getGuaranteedVoxel(0, maxY + 1, 0)->density = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(1, maxY + 1, 0)->density
		+ m_grid->getGuaranteedVoxel(0, maxY, 0)->density + m_grid->getGuaranteedVoxel(0, maxY + 1, 1)->density);
	//bottom-right-near
	m_grid->getGuaranteedVoxel(maxX + 1, 0, 0)->density = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(maxX, 0, 0)->density
		+ m_grid->getGuaranteedVoxel(maxX + 1, 1, 0)->density + m_grid->getGuaranteedVoxel(maxX + 1, 0, 1)->density);
	//right-top-near
	m_grid->getGuaranteedVoxel(maxX + 1, maxY + 1, 0)->density = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(maxX, maxY + 1, 0)->density
		+ m_grid->getGuaranteedVoxel(maxX + 1, maxY, 0)->density + m_grid->getGuaranteedVoxel(maxX + 1, maxY + 1, 1)->density);
	//left-bottom-far
	m_grid->getGuaranteedVoxel(0, 0, maxZ + 1)->density = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(1, 0, maxZ + 1)->density
		+ m_grid->getGuaranteedVoxel(0, 1, maxZ + 1)->density + m_grid->getGuaranteedVoxel(0, 0, maxZ)->density);
	//left-top-far
	m_grid->getGuaranteedVoxel(0, maxY + 1, maxZ + 1)->density = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(1, maxY + 1, maxZ + 1)->density
		+ m_grid->getGuaranteedVoxel(0, maxY, maxZ + 1)->density + m_grid->getGuaranteedVoxel(0, maxY + 1, maxZ)->density);
	//right-bottom-far
	m_grid->getGuaranteedVoxel(maxX + 1, 0, maxZ + 1)->density = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(maxX, 0, maxZ + 1)->density
		+ m_grid->getGuaranteedVoxel(maxX + 1, 1, maxZ + 1)->density + m_grid->getGuaranteedVoxel(maxX + 1, 0, maxZ)->density);
	//right-top-far
	m_grid->getGuaranteedVoxel(maxX + 1, maxY + 1, maxZ + 1)->density = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(maxX, maxY + 1, maxZ + 1)->density
		+ m_grid->getGuaranteedVoxel(maxX + 1, maxY, maxZ + 1)->density + m_grid->getGuaranteedVoxel(maxX + 1, maxY + 1, maxZ)->density);
	
}

void FluidSolver::force_boundries_preassure()
{
	unsigned int maxX = m_grid->getXMaxSize();
	unsigned int maxY = m_grid->getYMaxSize();
	unsigned int maxZ = m_grid->getZMaxSize();
	for (unsigned int i = 0; i <= maxX; i++)
	{
		for (unsigned int j = 0; j <= maxY; j++)
		{
			//near
			m_grid->getGuaranteedVoxel(i, j, 0)->preassure = m_grid->getGuaranteedVoxel(i, j, 1)->preassure;
			//far
			m_grid->getGuaranteedVoxel(i, j, maxZ)->preassure = m_grid->getGuaranteedVoxel(i, j, maxZ - 1)->preassure;
		}
	}
	for (unsigned int i = 0; i <= maxX; i++)
	{
		for (unsigned int j = 0; j < maxZ; j++)
		{
			//bottom
			m_grid->getGuaranteedVoxel(i, 0, j)->preassure = m_grid->getGuaranteedVoxel(i, 1, j)->preassure;
			//top
			m_grid->getGuaranteedVoxel(i, maxY, j)->preassure = m_grid->getGuaranteedVoxel(i, maxY - 1, j)->preassure;
		}
	}
	for (unsigned int i = 0; i < maxY; i++)
	{
		for (unsigned int j = 0; j < maxZ; j++)
		{
			//left
			m_grid->getGuaranteedVoxel(0, i, j)->preassure = m_grid->getGuaranteedVoxel(1, i, j)->preassure;
			//right
			m_grid->getGuaranteedVoxel(maxX, i, j)->preassure = m_grid->getGuaranteedVoxel(maxX - 1, i, j)->preassure;
		}
	}
	//left bottom near
	m_grid->getGuaranteedVoxel(0, 0, 0)->preassure = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(1, 0, 0)->preassure
		+ m_grid->getGuaranteedVoxel(0, 1, 0)->preassure + m_grid->getGuaranteedVoxel(0, 0, 1)->preassure);

	//top-left-near
	m_grid->getGuaranteedVoxel(0, maxY + 1, 0)->preassure = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(1, maxY + 1, 0)->preassure
		+ m_grid->getGuaranteedVoxel(0, maxY, 0)->preassure + m_grid->getGuaranteedVoxel(0, maxY + 1, 1)->preassure);
	//bottom-right-near
	m_grid->getGuaranteedVoxel(maxX + 1, 0, 0)->preassure = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(maxX, 0, 0)->preassure
		+ m_grid->getGuaranteedVoxel(maxX + 1, 1, 0)->preassure + m_grid->getGuaranteedVoxel(maxX + 1, 0, 1)->preassure);
	//right-top-near
	m_grid->getGuaranteedVoxel(maxX + 1, maxY + 1, 0)->preassure = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(maxX, maxY + 1, 0)->preassure
		+ m_grid->getGuaranteedVoxel(maxX + 1, maxY, 0)->preassure + m_grid->getGuaranteedVoxel(maxX + 1, maxY + 1, 1)->preassure);
	//left-bottom-far
	m_grid->getGuaranteedVoxel(0, 0, maxZ + 1)->preassure = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(1, 0, maxZ + 1)->preassure
		+ m_grid->getGuaranteedVoxel(0, 1, maxZ + 1)->preassure + m_grid->getGuaranteedVoxel(0, 0, maxZ)->preassure);
	//left-top-far
	m_grid->getGuaranteedVoxel(0, maxY + 1, maxZ + 1)->preassure = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(1, maxY + 1, maxZ + 1)->preassure
		+ m_grid->getGuaranteedVoxel(0, maxY, maxZ + 1)->preassure + m_grid->getGuaranteedVoxel(0, maxY + 1, maxZ)->preassure);
	//right-bottom-far
	m_grid->getGuaranteedVoxel(maxX + 1, 0, maxZ + 1)->preassure = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(maxX, 0, maxZ + 1)->preassure
		+ m_grid->getGuaranteedVoxel(maxX + 1, 1, maxZ + 1)->preassure + m_grid->getGuaranteedVoxel(maxX + 1, 0, maxZ)->preassure);
	//right-top-far
	m_grid->getGuaranteedVoxel(maxX + 1, maxY + 1, maxZ + 1)->preassure = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(maxX, maxY + 1, maxZ + 1)->preassure
		+ m_grid->getGuaranteedVoxel(maxX + 1, maxY, maxZ + 1)->preassure + m_grid->getGuaranteedVoxel(maxX + 1, maxY + 1, maxZ)->preassure);
}

void FluidSolver::force_boundries_divergence()
{
	unsigned int maxX = m_grid->getXMaxSize();
	unsigned int maxY = m_grid->getYMaxSize();
	unsigned int maxZ = m_grid->getZMaxSize();
	for (unsigned int i = 0; i <= maxX; i++)
	{
		for (unsigned int j = 0; j <= maxY; j++)
		{
			//near
			m_grid->getGuaranteedVoxel(i, j, 0)->divergence = m_grid->getGuaranteedVoxel(i, j, 1)->divergence;
			//far
			m_grid->getGuaranteedVoxel(i, j, maxZ)->divergence = m_grid->getGuaranteedVoxel(i, j, maxZ - 1)->divergence;
		}
	}
	for (unsigned int i = 0; i <= maxX; i++)
	{
		for (unsigned int j = 0; j < maxZ; j++)
		{
			//bottom
			m_grid->getGuaranteedVoxel(i, 0, j)->divergence = m_grid->getGuaranteedVoxel(i, 1, j)->divergence;
			//top
			m_grid->getGuaranteedVoxel(i, maxY, j)->divergence = m_grid->getGuaranteedVoxel(i, maxY - 1, j)->divergence;
		}
	}
	for (unsigned int i = 0; i < maxY; i++)
	{
		for (unsigned int j = 0; j < maxZ; j++)
		{
			//left
			m_grid->getGuaranteedVoxel(0, i, j)->divergence = m_grid->getGuaranteedVoxel(1, i, j)->divergence;
			//right
			m_grid->getGuaranteedVoxel(maxX, i, j)->divergence = m_grid->getGuaranteedVoxel(maxX - 1, i, j)->divergence;
		}
	}
	//left bottom near
	m_grid->getGuaranteedVoxel(0, 0, 0)->divergence = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(1, 0, 0)->divergence
		+ m_grid->getGuaranteedVoxel(0, 1, 0)->divergence + m_grid->getGuaranteedVoxel(0, 0, 1)->divergence);

	//top-left-near
	m_grid->getGuaranteedVoxel(0, maxY + 1, 0)->divergence = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(1, maxY + 1, 0)->divergence
		+ m_grid->getGuaranteedVoxel(0, maxY, 0)->divergence + m_grid->getGuaranteedVoxel(0, maxY + 1, 1)->divergence);
	//bottom-right-near
	m_grid->getGuaranteedVoxel(maxX + 1, 0, 0)->divergence = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(maxX, 0, 0)->divergence
		+ m_grid->getGuaranteedVoxel(maxX + 1, 1, 0)->divergence + m_grid->getGuaranteedVoxel(maxX + 1, 0, 1)->divergence);
	//right-top-near
	m_grid->getGuaranteedVoxel(maxX + 1, maxY + 1, 0)->divergence = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(maxX, maxY + 1, 0)->divergence
		+ m_grid->getGuaranteedVoxel(maxX + 1, maxY, 0)->divergence + m_grid->getGuaranteedVoxel(maxX + 1, maxY + 1, 1)->divergence);
	//left-bottom-far
	m_grid->getGuaranteedVoxel(0, 0, maxZ + 1)->divergence = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(1, 0, maxZ + 1)->divergence
		+ m_grid->getGuaranteedVoxel(0, 1, maxZ + 1)->divergence + m_grid->getGuaranteedVoxel(0, 0, maxZ)->divergence);
	//left-top-far
	m_grid->getGuaranteedVoxel(0, maxY + 1, maxZ + 1)->divergence = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(1, maxY + 1, maxZ + 1)->divergence
		+ m_grid->getGuaranteedVoxel(0, maxY, maxZ + 1)->divergence + m_grid->getGuaranteedVoxel(0, maxY + 1, maxZ)->divergence);
	//right-bottom-far
	m_grid->getGuaranteedVoxel(maxX + 1, 0, maxZ + 1)->divergence = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(maxX, 0, maxZ + 1)->divergence
		+ m_grid->getGuaranteedVoxel(maxX + 1, 1, maxZ + 1)->divergence + m_grid->getGuaranteedVoxel(maxX + 1, 0, maxZ)->divergence);
	//right-top-far
	m_grid->getGuaranteedVoxel(maxX + 1, maxY + 1, maxZ + 1)->divergence = (1.0f / 3.0f)*(
		m_grid->getGuaranteedVoxel(maxX, maxY + 1, maxZ + 1)->divergence
		+ m_grid->getGuaranteedVoxel(maxX + 1, maxY, maxZ + 1)->divergence + m_grid->getGuaranteedVoxel(maxX + 1, maxY + 1, maxZ)->divergence);
}

void FluidSolver::run(const float dt)
{
	addForce(glm::vec3(0, -9.81f, 0), dt);
	addSource(1.0f, dt);
	for (unsigned int i = 0; i < 5; i++)
	{
		velocity_step(dt);
		dens_step(dt);
	}
}

void FluidSolver::addForce(const glm::vec3& amount, const float dt)
{
	BEGIN_PER_CELL
		voxel* temp = m_grid->getGuaranteedVoxel(x, y, z);
		temp->velocity += dt*amount;
	END_PER_CELL
}
void FluidSolver::addSource(const float amount,const float dt)
{
	BEGIN_PER_CELL
		voxel* temp = m_grid->getVoxel(x, y, z);
		temp->density += dt*amount;
	END_PER_CELL
}

#undef BEGIN_PER_CELL
#undef END_PER_CELL
