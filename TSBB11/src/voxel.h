#include <vector>

struct voxel{
  float a;
  float b;
};


class Voxelgrid
{


public:
  Voxelgrid(int sizeX, int sizeY, int sizeZ);
  ~Voxelgrid();
  std::vector<voxel> *voxels;
  voxel* getData(int x,int y,int z);

private:

  voxel* V(int x,int y,int z);

  unsigned int numx;
  unsigned int numy;
  unsigned int numz;


};
