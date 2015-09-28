#include "voxel.h"


/* -----------------------------------------------------------------
* Malloc the whole array, based on size(xyz);
* In this implementation, x,z represent the width and depth, and y is the height.
*/

Voxelgrid::Voxelgrid(int sizeX, int sizeY, int sizeZ){


  //Assign the new vector with all elements set to the voxel structs
  this->voxels  = new std::vector<voxel>(sizeX*sizeY*sizeZ);

  //Assign dimensions of the grid
  numx = sizeX;
  numy = sizeY;
  numz = sizeZ;

}

/* -----------------------------------------------------------------
*  Return data at, x,y,z (complete struct)
*  Note that since this function returns a pointer, modifications to the
*  data inside the struct is possible.
*  This can be used as a setter for the members of the struct.
*/

voxel* Voxelgrid::getData(int x,int y,int z){
  return V(x,y,z);
}

/* -----------------------------------------------------------------
* Here setters will be present when which datatypes that should be used
* is determined.
*/





/* -----------------------------------------------------------------
* Explicitly delete the vector since it occupies alot of memory.
*/

Voxelgrid::~Voxelgrid(){

  delete voxels;
}

/* -----------------------------------------------------------------
*  Accessor for x,y,z to linear access in the vector. Will throw if
* trying to access outside of gridsize.
*/

voxel* Voxelgrid::V(int x, int y, int z){
  if( x > numx || y > numy || z > numz){
    throw('Trying to access data outside of voxelgrid index');
    return nullptr;
  }
  return &(voxels->at(x + numx * (z + numz * y)));


}
