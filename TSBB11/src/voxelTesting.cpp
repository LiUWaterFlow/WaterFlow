#include "voxelTesting.h"
#include <time.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

namespace voxelTest{

  int parseLine(char* line){
    int i = strlen(line);
    while (*line < '0' || *line > '9') line++;
    line[i-3] = '\0';
    i = atoi(line);
    return i;
  }


  int getValue(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];


    while (fgets(line, 128, file) != NULL){
      if (strncmp(line, "VmSize:", 7) == 0){
        result = parseLine(line);
        break;
      }
    }
    fclose(file);
    return result;
  }



  clock_t begin_time;

  void startClock(){
    begin_time = clock();

  }
  void endClock(){
    std::cout << float( clock () - begin_time ) /  CLOCKS_PER_SEC << std::endl;
  }

  VoxelTest::VoxelTest(DataHandler* inDataPtr,Voxelgrid* inGridPtr){
    dataPtr = inDataPtr;
    gridPtr = inGridPtr;
  }

  void mainTest(VoxelTest* tester){
    Voxelgrid* grid = tester->gridPtr;
    startClock();
    size_t count = 500;
    size_t end = 0;
    for (size_t x = count; x != end; x--) {
      for (size_t y = count; y != end; y--) {
        for (size_t z = count; z != end; z--) {
          grid->setVoxel(x,y,z,1,x,x);
        }
      }
    }
    endClock();


    //Read and modify the voxels
    startClock();
    for (size_t x = count*2; x != end; x--) {
      for (size_t y = count*2; y != end; y--) {
        for (size_t z = count*2; z != end; z--) {
          voxel* tmp = grid->getVoxel(x,y,z);
          if(tmp != nullptr)
            tmp->a++;
        }
      }
    }
    endClock();
    //Delete the voxels

    //Read some voxels that doesn't exist


  }

}
