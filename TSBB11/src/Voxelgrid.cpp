#include "Voxelgrid.h"


/* -----------------------------------------------------------------
Voxelgrid - Create the initial vector strutcture.
*/

Voxelgrid::Voxelgrid(DataHandler* dataHandler){

  this->voxels  = new std::vector<std::vector<std::vector<Voxel*>*>*>;

  this->datahandler = dataHandler;
}


/* -----------------------------------------------------------------
Voxelgrid - Destructor, ensure destruction of all pointer structures
*/

Voxelgrid::~Voxelgrid(){
  for (unsigned int x = 0; x < voxels->size(); x++) {
    if(voxels->at(x) != nullptr){
      for (unsigned int y = 0; y < voxels->at(x)->size(); y++) {
        if(voxels->at(x)->at(y) != nullptr){
          for (unsigned int z = 0; z < voxels->at(x)->at(y)->size(); z++) {
            if( voxels->at(x)->at(y)->at(z) != nullptr){
              delete voxels->at(x)->at(y)->at(z);
            }
          }
          delete voxels->at(x)->at(y);
        }
      }
      delete voxels->at(x);
    }
  }
delete voxels;
}
void Voxelgrid::createVoxel(int x, int y, int z)
{
	makeSpaceForVoxel(x, y, z);
	Voxel* temp = new Voxel;
	voxels->at(x)->at(y)->at(z) = temp;
}

/* -----------------------------------------------------------------
setVoxel take a x,y,z coordinate where the Voxel will be created (or modified)
with the values ax, bx.
*/

void Voxelgrid::setVoxel(int x,int y,int z, bool filledx, float ax,float bx){

	makeSpaceForVoxel(x, y, z);
  //create and insert the new Voxel.
  Voxel* temp = new Voxel;
  voxels->at(x)->at(y)->at(z) = temp;

}
/*
makeSpaceForVoxel takes a x,y,z coordinate where the grid will resize itself if needed
and remove the Voxel if it already exists, in preperation for inserting a new one.
*/

void Voxelgrid::makeSpaceForVoxel(int x, int y, int z)
{

	//if x is not in table. Create y and z tables, resize x, and
	//point to children (y,z);
	if (voxels->size() < (unsigned int)(x + 1)) {
		std::vector<Voxel*>* tempZ = new std::vector<Voxel*>(z + 1);
		std::vector<std::vector<Voxel*>*>* tempY = new std::vector<std::vector<Voxel*>*>(y + 1);
		voxels->resize(x + 1, nullptr);
		voxels->at(x) = tempY;
		voxels->at(x)->at(y) = tempZ;

	}
	//if y is not in table. Create z table, resize y, and
	//point to childtable z; Note that existence of y table is
	//managed by the first part of the if-statement
	else if (voxels->at(x) != nullptr && voxels->at(x)->size() < (unsigned int) (y + 1)) {

		std::vector<Voxel*>* tempZ = new std::vector<Voxel*>(z + 1);
		voxels->at(x)->resize(y + 1, nullptr);
		voxels->at(x)->at(y) = tempZ;

	}
	//if z is not large enough resize. Note that existence of z table is
	//managed by the first two parts of the if-statement
	else if (voxels->at(x) != nullptr &&  voxels->at(x)->at(y) != nullptr && voxels->at(x)->at(y)->size() < (unsigned int)(z + 1)) {
		voxels->at(x)->at(y)->resize(z + 1, nullptr);

	}

	//If x is large enough but does not contain a table at position x
	//create and insert relevant tables.
	if (voxels->at(x) == nullptr) {
		std::vector<Voxel*>* tempZ = new std::vector<Voxel*>(z + 1);
		std::vector<std::vector<Voxel*>*>* tempY = new std::vector<std::vector<Voxel*>*>(y + 1);
		voxels->at(x) = tempY;
		voxels->at(x)->at(y) = tempZ;
	}
	//If y is large enough but does not contain a table at position y
	//create and insert relevant tables.
	else if (voxels->at(x)->at(y) == nullptr) {
		std::vector<Voxel*>* tempZ = new std::vector<Voxel*>(z + 1);
		voxels->at(x)->at(y) = tempZ;
	}
	//If there is already a Voxel at position x,y,z, delete this in
	//preperation for new insertion.
	else if (voxels->at(x)->at(y)->at(z) != nullptr) {
		delete voxels->at(x)->at(y)->at(z);
	}
}

/* -----------------------------------------------------------------
Get Voxel at x,y,z. This function returns a pointer to the struct.
If no Voxel is present it returns a nullptr.
*/

Voxel* Voxelgrid::getVoxel(int x,int y,int z){
  //std::cout << "In getVoxel, size of vector voxels is: " << voxels->size() << std::endl;

  //std::cout << "Voxels at x is empty" << std::endl;

  //ensure table existance and table size, if either fails return nullptr.
  if(voxels->size() < (unsigned int)(x+1) || voxels->at(x) == nullptr){
    //std::cout << "In first if in get_Voxel" << std::endl;
    return nullptr;
  }
  //ensure table existance and table size, if either fails return nullptr.
  else if(voxels->at(x)->size() < (unsigned int) (y+1) || voxels->at(x)->at(y) == nullptr){
    //std::cout << "In second if in get_Voxel" << std::endl;
    return nullptr;
  }
  //ensure table existance and table size, if either fails return nullptr.
  else if(voxels->at(x)->at(y)->size() < (unsigned int) (z+1) || voxels->at(x)->at(y)->at(z) == nullptr){
    return nullptr;
  }

  //Existance is ensured, return pointer at location x,y,z
  //std::cout << "Just before returning Voxel in get_Voxel" << std::endl;
  return voxels->at(x)->at(y)->at(z);
}


/* FloodFill function creates a vector queue. Tests if the first Voxel coordinates are above land, if so its coordinates are added to the queue
vector and the struct for the Voxel is creatd using setVoxel. While there are still coordinates in the queue, the neighboring voxels'
coordinates relative to the current coordinates (last position in queue) are added to the queue and a corresponding struct is created with setVoxel.
As each element in the queue is processed the voxels beneath the current Voxel are filled.
*/

void Voxelgrid::FloodFill(int init_x, int height, int init_z){

  std::vector<std::vector<int>> queue;

  if (datahandler->getCoord(init_x, init_z) < height) {
    queue.push_back({init_x, init_z});
    setVoxel(init_x, height, init_z, true, 0, 0);
  }

  int temp_x, temp_z;
  int height_test;
  int terrain_height;

/* While queue is not empty, keep processing queue from back to front.
*/
  while(queue.size() > 0){

    temp_x  = queue.back().at(0);
    temp_z = queue.back().at(1);

    queue.pop_back();

    /* Fill voxels beneath current Voxel
    */
    height_test = height;
    terrain_height = (int)datahandler->getCoord(temp_x, temp_z);

    while(height_test > terrain_height && height_test >= 0){

      setVoxel(temp_x, height_test, temp_z, true, 0, 0);
      height_test--;
    }

/* Checking voxels adjacent to current Voxel and adding their coordinates to the queue if they are inside the terrain,
above land and have not yet been added to the queue. Before coordina are added the struct is created. Struct existance
(!= nullptr) thus equivalent to Voxel added to cue as used in if-statement.
*/

    if(temp_x + 1 < datahandler->getWidth() &&
        datahandler->getCoord(temp_x + 1, temp_z) < height &&
          getVoxel(temp_x + 1, height, temp_z) == nullptr){
      setVoxel(temp_x + 1, height, temp_z, true, 0, 0);
      queue.push_back({temp_x + 1, temp_z});
    }
    if(temp_x - 1 >= 0 &&
        datahandler->getCoord(temp_x - 1, temp_z) < height &&
          getVoxel(temp_x - 1, height, temp_z) == nullptr){
      setVoxel(temp_x - 1, height, temp_z, true, 0, 0);
      queue.push_back({temp_x - 1, temp_z});
    }
    if(temp_z < datahandler->getWidth() &&
        datahandler->getCoord(temp_x, temp_z + 1) < height &&
          getVoxel(temp_x, height, temp_z + 1) == nullptr){
      setVoxel(temp_x, height, temp_z + 1, true, 0, 0);
      queue.push_back({temp_x, temp_z + 1});
    }
    if(temp_z <= 0 &&
        datahandler->getCoord(temp_x, temp_z - 1) < height &&
          getVoxel(temp_x, height, temp_z - 1) == nullptr){
      setVoxel(temp_x, height, temp_z - 1, true, 0, 0);
      queue.push_back({temp_x, temp_z - 1});
    }
  }
}

NeighbourVoxels Voxelgrid::getNeighbour(int x, int y, int z)
{
	assert((x - 1) > 0 && (y - 1) > 0 && (z - 1) > 0);
	NeighbourVoxels vox;
	vox.voxels[CUBEPOS::FAR_TOP_LEFT] = getGuaranteedVoxel(x - 1, y + 1, z - 1);
	vox.voxels[CUBEPOS::FAR_TOP_CENTER] = getGuaranteedVoxel(x , y + 1, z - 1);
	vox.voxels[CUBEPOS::FAR_TOP_RIGHT] = getGuaranteedVoxel(x + 1, y + 1, z - 1);

	vox.voxels[CUBEPOS::FAR_MID_LEFT] = getGuaranteedVoxel(x - 1, y, z - 1);
	vox.voxels[CUBEPOS::FAR_MID_CENTER] = getGuaranteedVoxel(x, y, z - 1);
	vox.voxels[CUBEPOS::FAR_MID_RIGHT] = getGuaranteedVoxel(x + 1, y, z - 1);

	vox.voxels[CUBEPOS::FAR_BOTTOM_LEFT] = getGuaranteedVoxel(x - 1, y - 1, z - 1);
	vox.voxels[CUBEPOS::FAR_BOTTOM_CENTER] = getGuaranteedVoxel(x, y - 1, z - 1);
	vox.voxels[CUBEPOS::FAR_BOTTOM_RIGHT] = getGuaranteedVoxel(x + 1, y - 1, z - 1);

	vox.voxels[CUBEPOS::CURRENT_TOP_LEFT] = getGuaranteedVoxel(x - 1, y + 1, z);
	vox.voxels[CUBEPOS::CURRENT_TOP_CENTER] = getGuaranteedVoxel(x, y + 1, z);
	vox.voxels[CUBEPOS::CURRENT_TOP_RIGHT] = getGuaranteedVoxel(x + 1, y + 1, z);

	vox.voxels[CUBEPOS::CURRENT_MID_LEFT] = getGuaranteedVoxel(x - 1, y, z);
	vox.voxels[CUBEPOS::CURRENT_MID_CENTER] = getGuaranteedVoxel(x, y, z);
	vox.voxels[CUBEPOS::CURRENT_MID_RIGHT] = getGuaranteedVoxel(x + 1, y, z);

	vox.voxels[CUBEPOS::CURRENT_BOTTOM_LEFT] = getGuaranteedVoxel(x - 1, y - 1, z);
	vox.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER] = getGuaranteedVoxel(x, y - 1, z);
	vox.voxels[CUBEPOS::CURRENT_BOTTOM_RIGHT] = getGuaranteedVoxel(x + 1, y - 1, z);

	vox.voxels[CUBEPOS::NEAR_TOP_LEFT] = getGuaranteedVoxel(x - 1, y + 1, z + 1);
	vox.voxels[CUBEPOS::NEAR_TOP_CENTER] = getGuaranteedVoxel(x, y + 1, z + 1);
	vox.voxels[CUBEPOS::NEAR_TOP_RIGHT] = getGuaranteedVoxel(x + 1, y + 1, z + 1);

	vox.voxels[CUBEPOS::NEAR_MID_LEFT] = getGuaranteedVoxel(x - 1, y, z + 1);
	vox.voxels[CUBEPOS::NEAR_MID_CENTER] = getGuaranteedVoxel(x, y, z + 1);
	vox.voxels[CUBEPOS::NEAR_MID_RIGHT] = getGuaranteedVoxel(x + 1, y, z + 1);

	vox.voxels[CUBEPOS::NEAR_BOTTOM_LEFT] = getGuaranteedVoxel(x - 1, y - 1, z + 1);
	vox.voxels[CUBEPOS::NEAR_BOTTOM_CENTER] = getGuaranteedVoxel(x, y - 1, z + 1);
	vox.voxels[CUBEPOS::NEAR_BOTTOM_RIGHT] = getGuaranteedVoxel(x + 1, y - 1, z + 1);

	return vox;
}

Voxel* Voxelgrid::getGuaranteedVoxel(int x, int y, int z)
{
	Voxel* temp = getVoxel(x, y, z);
	if (temp == nullptr)
	{
		createVoxel(x, y, z);
	}
	return getVoxel(x, y, z);
}