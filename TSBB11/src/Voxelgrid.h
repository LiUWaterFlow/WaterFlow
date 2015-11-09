#ifndef VOXELGRID_H
#define VOXELGRID_H


#include <vector>
#include <array>
#include <iostream>
#include "readData.h"
#include "fluidDataStructures.h"

/// @class Voxelgrid
/// @brief Handles the representation of the voxelgrid.
///
/// The class is used to construct the voxelgrid used for simulation and
/// visualization. It places a Voxel for each point in the base area (x times z)
/// and uses N number of voxels in the precision from the lowest point in the model
/// to the heighest peak. Note voxels outside of highest possible, however not
/// negative Voxel coordinates. The class implements a rudimentary sparse voxelgrid.
class Voxelgrid
{
private:
  std::vector<std::vector<std::vector<Voxel*>*>*>* voxels; ///< Container for the Voxel lookup tables
  DataHandler* datahandler; ///< Handle to the datahandler and thus the model data.

  void makeSpaceForVoxel(int x, int y, int z); //resizes and removes Voxel at pos x y z

public:

	Voxelgrid() {};

  /// @brief Constructs a empty Voxel grid
  ///
  /// Constructs an initially empty sparse voxelgrid, which scales so that there
  /// is N number of voxels in height representing the lowest point to the heighest.
  /// Where N is ceil(DataHandler->getTerrainScale)
  /// The grid dynamically grows when voxels are added.
  /// @param handle Handle to the DataHandler will be bound to the class
  /// @see DataHandle::getTerrainScale
  /// @see setVoxel
  /// @see getVoxel
  /// @todo The Voxelgrid should be able to handle one Voxel outside the area of the terrain in each dimension.
  Voxelgrid(DataHandler* handle);


  /// @brief Completely delete the sparse voxelgrid.
  ~Voxelgrid();

  /// @brief Set value of Voxel at x,y,z.
  ///
  /// Sets the values of the Voxel at x,y,z and creates it in the sparse voxelgrid.
  /// if setting many voxels at the same time, loop over z first, followed by y,
  /// and from go from HIGH values towards LOW. (minimizes resizing)
  /// @param x Coordinate of the Voxel, x cannot be negative.
  /// @param y Coordinate of the Voxel, y cannot be negative.
  /// @param z Coordinate of the Voxel, z cannot be negative.
  /// @param filled Bool determining if the Voxel has been filled or not.
  /// @param a Dummy variable for initial tests of size and performance
  /// @param b Dummy variable for initial tests of size and performance
  /// @see getVoxel
  void setVoxel(int x,int y,int z, bool filled, float a,float b);

  /// @brief Creates a default initialized Voxel at x,y,z
  ///
  /// Creates a Voxel at position x, y, z in the sparse voxelgrid with default values
  /// if creating many voxels at the same time, loop over z first, followed by y,
  /// and from go from HIGH values towards LOW. (minimizes resizing)
  /// @param x Coordinate of the Voxel, x cannot be negative.
  /// @param y Coordinate of the Voxel, y cannot be negative.
  /// @param z Coordinate of the Voxel, z cannot be negative.
  /// @see getVoxel
  void createVoxel(int x, int y, int z);

  /// @brief Get value of Voxel at x,y,z.
  ///
  /// Sets the values of the Voxel at x,y,z and creates it in the sparse voxelgrid.
  /// if setting many voxels at the same time, loop over z first, followed by y,
  /// and from go from LOW values towards HIGH. (maximizes cache hits)
  /// @param x Coordinate of the Voxel, x cannot be negative.
  /// @param y Coordinate of the Voxel, y cannot be negative.
  /// @param z Coordinate of the Voxel, z cannot be negative.
  /// @see setVoxel
  /// @return Returns a pointer to the Voxel (i.e. changes can be made.)
  Voxel* getVoxel(int x,int y,int z);

  Voxel* getGuaranteedVoxel(int x, int y, int z);
  
  NeighbourVoxels getNeighbour(int x, int y, int z);

  unsigned int getXMaxSize() { return 10; }

  unsigned int getYMaxSize() { return 10; }

  unsigned int getZMaxSize() { return 10; }

  std::vector<Voxel*> getBorderTop();

  std::vector<Voxel*> getBorderBottom();

  std::vector<Voxel*> getBorderLeft();

  std::vector<Voxel*> getBorderRight();

  std::vector<Voxel*> getBorderFront();

  std::vector<Voxel*> getBorderBack();

  std::vector<std::array<int, 2>>* LayerFloodFill(int init_x, int init_z, int height);
  void LayerFloodFill_Rec(int x, int z, int height, std::vector<std::array<int, 2>>* filled_coords);
  void FloodFill(int init_x, int init_z, int height);




};

#endif //VOXELGRID_H
