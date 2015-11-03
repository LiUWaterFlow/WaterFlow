#include "voxelTesting.h"
#include <time.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <cstdlib>

namespace voxelTest{

  void plsWait(){
    
#ifdef _WINDOWS
    system("pause");
#else
	std::cout << "Press any key to continue..." << std::endl;
    system("read");
#endif
  }

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
    size_t count = 300;
    size_t end = 0;
	bool testHash = false;

	if (testHash) {
		//plsWait();

		grid->hashInit();

		for (size_t x = count; x != end; x--) {
			for (size_t y = count; y != end; y--) {
				for (size_t z = count; z != end; z--) {
					grid->hashAdd(x, y, z, 1, x, y);
				}
			}
		}
		endClock();

		std::cout << (float)grid->numCollisions/(float)grid->numInTable << std::endl;

		//Read and modify the voxels
		startClock();
		for (size_t x = count; x != end; x--) {
			for (size_t y = count; y != end; y--) {
				for (size_t z = count; z != end; z--) {
					voxel* tmp = grid->hashGet(x, y, z);
					if (tmp != nullptr)
						tmp->a += 1;
				}
			}
		}
		printf("\n");
		endClock();
	
	}
	else{

    for (size_t x = count; x != end; x--) {
      for (size_t y = count; y != end; y--) {
        for (size_t z = count; z != end; z--) {
          grid->setVoxel(x,y,z,1,x,y);
        }
      }
    }
    endClock();

    //Read and modify the voxels
    startClock();
	for (size_t x = count; x != end; x--) {
		for (size_t y = count; y != end; y--) {
			for (size_t z = count; z != end; z--) {
				voxel* tmp = grid->getVoxel(x, y, z);
				if(tmp != nullptr)
					tmp->a += 1;
			}
		}
	}
    printf("\n");
    endClock();

	}
    //Delete the voxels
    plsWait();
    delete grid;
    plsWait();
    //Read some voxels that doesn't exist


  }

}
