#include "voxelTesting.h"
#include <time.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <cstdlib>

#ifdef _WINDOWS
#include <Windows.h>
#include <iostream>
#include <sstream>

#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s;                   \
   OutputDebugStringW( os_.str().c_str() );  \
}

#else
#define DBOUT( s ) \
{					\
}					\
#endif
#endif

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
  void endClock() {
	float time = float(clock() - begin_time) / CLOCKS_PER_SEC;
	DBOUT("Value of time: " << time << std::endl);
	std::cout << time << std::endl;
  }


  int randi(int max, int min) {
	  return min + (rand() % (int)(max - min + 1));
  }

  VoxelTest::VoxelTest(DataHandler* inDataPtr,Voxelgrid* inGridPtr){
    dataPtr = inDataPtr;
    gridPtr = inGridPtr;
  }

  void mainTest(DataHandler* data) {

	  Voxelgrid* grid = new Voxelgrid(data, pow(2, 26));


	  startClock();
	  size_t count = 350;
	  size_t end = 0;
	  bool testHash = !true;
	  bool randomRead = !true;
	  bool neighbours = !true;
	  int xd, yd, zd;

	  srand(2500);

	  startClock();
	  if (randomRead){
		  for (size_t x = count; x != end; x--) {
			  for (size_t y = count; y != end; y--) {
				  for (size_t z = count; z != end; z--) {

					  xd = randi(count, end);
					  yd = randi(count, end);
					  zd = randi(count, end);
				  }
			  }
		  }
	  }
	  endClock();

	  srand(2500);

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
					
					if (randomRead) {
						xd = randi(count, end);
						yd = randi(count, end);
						zd = randi(count, end);
					}
					else {
						xd = x;
						yd = y;
						zd = z;
					}

					if (neighbours) {
						neighs* tmp = grid->getNeighbourhoodHash(xd, yd, zd);
						if (tmp != nullptr)
							tmp->voxs[0]->a += 1;

						delete tmp;
					}
					else {
						voxel* tmp = grid->hashGet(xd, yd, zd);
						if (tmp != nullptr)
							tmp->a += 1;
					}
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


				if (randomRead) {
					xd = randi(count, end);
					yd = randi(count, end);
					zd = randi(count, end);
				}
				else {
					xd = x;
					yd = y;
					zd = z;
				}
				if (neighbours) {
					neighs* tmp = grid->getNeighbourhood(xd, yd, zd);
					if (tmp != nullptr)
						tmp->voxs[0]->a += 1;

					delete tmp;
				}
				else {
					voxel* tmp = grid->getVoxel(xd, yd, zd);
					if (tmp != nullptr)
						tmp->a += 1;
				}
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
