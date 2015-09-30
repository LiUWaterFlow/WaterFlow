#include <vector>
#include <iostream>

struct voxel{
  float a;
  float b;
};


class Voxelgrid
{


public:
  Voxelgrid();
  ~Voxelgrid();
  void setVoxel(int x,int y,int z,float a,float b);
  voxel* getVoxel(int x,int y,int z);

private:


  std::vector<std::vector<std::vector<voxel*>*>*>* voxels;
  //voxel* V(int x,int y,int z);

  unsigned int numx;
  unsigned int numy;
  unsigned int numz;


};
