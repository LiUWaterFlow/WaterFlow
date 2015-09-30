#include "voxel.h"




/* -----------------------------------------------------------------
Voxelgrid - Create the initial vector strutcture.
*/

Voxelgrid::Voxelgrid(){

  this->voxels  = new std::vector<std::vector<std::vector<voxel*>*>*>;

}


/* -----------------------------------------------------------------
Voxelgrid - Destructor, ensure destruction of all pointer structure
*/

Voxelgrid::~Voxelgrid(){
  for (int x = 0; x < voxels->size(); x++) {
    if(voxels->at(x) != nullptr){
      for (int y = 0; y < voxels->at(x)->size(); y++) {
        if(voxels->at(x)->at(y) != nullptr){
          for (int z = 0; z < voxels->at(x)->at(y)->size(); z++) {
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



/* -----------------------------------------------------------------
setVoxel take a x,y,z coordinate where the voxel will be created (or modified)
with the values ax, bx.
*/

void Voxelgrid::setVoxel(int x,int y,int z,float ax,float bx){



  //if x is not in table. Create y and z tables, resize x, and
  //point to children (y,z);
  if(voxels->size() < x+1){
    std::vector<voxel*>* tempZ = new std::vector<voxel*>(z+1);
    std::vector<std::vector<voxel*>*>* tempY = new std::vector<std::vector<voxel*>*>(y+1);
    voxels->resize(x+1,nullptr);
    voxels->at(x) = tempY;
    voxels->at(x)->at(y) = tempZ;

  }
  //if y is not in table. Create z table, resize y, and
  //point to childtable z; Note that existence of y table is
  //managed by the first part of the if-statement
  else if(voxels->at(x) != nullptr && voxels->at(x)->size() < y+1){

    std::vector<voxel*>* tempZ = new std::vector<voxel*>(z+1);
    voxels->at(x)->resize(y+1,nullptr);
    voxels->at(x)->at(y) = tempZ;

  }
  //if z is not large enough resize. Note that existence of z table is
  //managed by the first two parts of the if-statement
  else if(voxels->at(x) != nullptr &&  voxels->at(x)->at(y) != nullptr && voxels->at(x)->at(y)->size() < z+1){
    voxels->at(x)->at(y)->resize(z+1,nullptr);

  }

  //If x is large enough but does not contain a table at position x
  //create and insert relevant tables.
  if(voxels->at(x) == nullptr){
    std::vector<voxel*>* tempZ = new std::vector<voxel*>(z+1);
    std::vector<std::vector<voxel*>*>* tempY = new std::vector<std::vector<voxel*>*>(y+1);
    voxels->at(x) = tempY;
    voxels->at(x)->at(y) = tempZ;
  }
  //If y is large enough but does not contain a table at position y
  //create and insert relevant tables.
  else if(voxels->at(x)->at(y) == nullptr){
    std::vector<voxel*>* tempZ = new std::vector<voxel*>(z+1);
    voxels->at(x)->at(y) = tempZ;
  }
  //If there is already a voxel at position x,y,z, delete this in
  //preperation for new insertion.
  else if(voxels->at(x)->at(y)->at(z)  != nullptr){
    delete voxels->at(x)->at(y)->at(z);
  }

  //create and insert the new voxel.
  voxel* temp = new voxel;
  temp->a = ax;
  temp->b = bx;
  voxels->at(x)->at(y)->at(z) = temp;
}

/* -----------------------------------------------------------------
Get voxel at x,y,z. This function returns a pointer to the struct.
If no voxel is present it returns a nullptr.
*/

voxel* Voxelgrid::getVoxel(int x,int y,int z){

  //ensure table existance and table size, if either fails return nullptr.
  if(voxels->at(x) == nullptr && voxels->size() < x+1){
    return nullptr;
  }
  //ensure table existance and table size, if either fails return nullptr.
  else if(voxels->at(x)->at(y) == nullptr && voxels->at(x)->size() < y+1){
    return nullptr;
  }
  //ensure table existance and table size, if either fails return nullptr.
  else if(voxels->at(x)->at(y)->at(z) == nullptr && voxels->at(x)->at(y)->size() < z+1){
    return nullptr;
  }

  //Existance is ensured, return pointer at location x,y,z
  return voxels->at(x)->at(y)->at(z);
}
