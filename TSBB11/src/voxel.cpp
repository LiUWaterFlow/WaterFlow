#include "voxel.h"




/* -----------------------------------------------------------------
Voxelgrid - Create the initial vector strutcture.
*/

Voxelgrid::Voxelgrid(DataHandler* dataHandler){

  this->voxels  = new std::vector<std::vector<std::vector<voxel*>*>*>;

  this->datahandler = dataHandler;
}


/* -----------------------------------------------------------------
Voxelgrid - Destructor, ensure destruction of all pointer structures
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

void Voxelgrid::setVoxel(int x,int y,int z, bool filledx, float ax,float bx){



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
  temp->filled = filledx;
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

/* LayerFloodFill is called from FloodFill and it creates a vector that stores the 2D coords of the filled voxels and then initiates the recursive
LayerFloodFill_Rec function.
*/

std::vector<std::array<int, 2>>* Voxelgrid::LayerFloodFill(int init_x, int init_z, int height){

  if (height > datahandler->getCoord(init_x, init_z)) {
    std::vector<std::array<int, 2>>* filled_coords = new std::vector<std::array<int, 2>>;
    LayerFloodFill_Rec(init_x, height, init_z, filled_coords);
    return filled_coords;
  }
  else{
    return nullptr;
  }
}

/*LayerFloodFill_Rec fills the inital voxel if it's not already filled and then calls itself on the adjacent four voxels.
*/

void Voxelgrid::LayerFloodFill_Rec(int x, int z, int height, std::vector<std::array<int, 2>>* filled_coords){

  if (!(getVoxel(x, height, z)->filled)) {

    setVoxel(x, height, z, true, 0, 0);

    filled_coords->push_back({x,z});

    LayerFloodFill_Rec((x+1), height, z, filled_coords);
    LayerFloodFill_Rec((x-1), height, z, filled_coords);
    LayerFloodFill_Rec(x, height, (z+1), filled_coords);
    LayerFloodFill_Rec(x, height, (z-1), filled_coords);
    }

  return;
}

/* FloodFill function initiates the filled_coords vector which is assigned values by later called LayerFloodFill and LayerFloodFill_Rec
functions. When the LayerFloodFill is finished the function iterates over all the filled coordinates in the floodfilled layer and fills
voxels beneath down to the height of the terrain.
*/

void Voxelgrid::FloodFill(int init_x, int init_z, int height){

/* 2D flood fill layer "height"
*/
  std::vector<std::array<int, 2>>* filled_coords;

  filled_coords = LayerFloodFill(init_x, init_z,  height);

/* Fill downwards
*/
  if (filled_coords != nullptr) {

    for (size_t i = 0; i < filled_coords->size(); i++) {

      int height_check = height - 1;
      int tmp_x = filled_coords->at(i).at(1);
      int tmp_z = filled_coords->at(i).at(2);
      while (height_check > datahandler->getCoord(tmp_x, tmp_z)){

        setVoxel(tmp_x, height_check, tmp_z, true, 0, 0);
        height_check--;
      }
    }
  }
}
