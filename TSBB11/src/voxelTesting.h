#ifndef VOXELTESTING_H
#define VOXELTESTING_H

#include "voxelGrid.h"
#include "readData.h"
#include "glm.hpp"
#include <iostream>
#include <cstdlib>

namespace voxelTest{

  class VoxelTest{
  public:
    DataHandler* dataPtr;
    Voxelgrid* gridPtr;

    VoxelTest(DataHandler* inDataPtr,Voxelgrid* inGridPtr);
  };


  //Public loose functions in the testing namespace.
  void mainTest(VoxelTest*);
}




#endif
