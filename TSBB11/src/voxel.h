#include <vector>
#include <array>
#include <iostream>
#include "readData.h"

struct voxel{
  bool filled;
  float a;
  float b;
};


class Voxelgrid
{


public:
  Voxelgrid(DataHandler*);
  ~Voxelgrid();
  void setVoxel(int x,int y,int z, bool filled, float a,float b);
  voxel* getVoxel(int x,int y,int z);

  std::vector<std::array<int, 2>>* LayerFloodFill(int init_x, int init_z, int height);
  void LayerFloodFill_Rec(int x, int z, int height, std::vector<std::array<int, 2>>* filled_coords);
  void FloodFill(int init_x, int init_z, int height);

private:

  std::vector<std::vector<std::vector<voxel*>*>*>* voxels;
  //voxel* V(int x,int y,int z);
  DataHandler* datahandler;

  unsigned int numx;
  unsigned int numy;
  unsigned int numz;


};
