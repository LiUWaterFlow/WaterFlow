#ifndef VOXELTESTING_H
#define VOXELTESTING_H

#include "grid.h"
#include "readData.h"
#include "glm.hpp"
#include <iostream>
#include <cstdlib>




namespace voxelTest{

void plsWait();
void startClock();
void endClock();

  class VoxelTest{
  public:
    DataHandler* dataPtr;
    Voxelgrid* gridPtr;

    VoxelTest(DataHandler* inDataPtr,Voxelgrid* inGridPtr);
  };


  //Public loose functions in the testing namespace.
  void mainTest(DataHandler* data);
}




#endif
