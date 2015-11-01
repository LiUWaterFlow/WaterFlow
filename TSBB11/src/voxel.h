#ifndef VOXEL_H
#define VOXEL_H


#include <vector>
#include <array>
#include "readData.h"

/// @strcut voxel
/// @brief Contains information and data in each voxel.
///

struct voxel{
  bool filled;  ///<The fill state of the voxel (maybe not needed)
  float a;      ///<Float a (temporary for space measurements and testing)
  float b;      ///<Float b (temporary for space measurements and testing)
};

/// @class Voxelgrid
/// @brief Handles the representation of the voxelgrid.
///
/// The class is used to construct the voxelgrid used for simulation and
/// visualization. It places a voxel for each point in the base area (x times z)
/// and uses N number of voxels in the precision from the lowest point in the model
/// to the heighest peak. Note voxels outside of highest possible, however not
/// negative voxel coordinates. The class implements a rudimentary sparse voxelgrid.
class Voxelgrid
{
private:
  std::vector<std::vector<std::vector<voxel*>*>*>* voxels; ///< Container for the voxel lookup tables
  DataHandler* datahandler; ///< Handle to the datahandler and thus the model data.

  std::vector<GLuint> *voxelPositions = nullptr;
  GLuint numVoxels;
  GLuint voxelShader;
  GLuint voxelBuffer, voxelVAO;

public:

  /// @brief Constructs a empty voxel grid
  ///
  /// Constructs an initially empty sparse voxelgrid, which scales so that there
  /// is N number of voxels in height representing the lowest point to the heighest.
  /// Where N is ceil(DataHandler->getTerrainScale)
  /// The grid dynamically grows when voxels are added.
  /// @param handle Handle to the DataHandler will be bound to the class
  /// @see DataHandle::getTerrainScale
  /// @see setVoxel
  /// @see getVoxel
  /// @todo The Voxelgrid should be able to handle one voxel outside the area of the terrain in each dimension.
  Voxelgrid(DataHandler* handle);


  /// @brief Completely delete the sparse voxelgrid.
  ~Voxelgrid();

  /// @brief Set value of voxel at x,y,z.
  ///
  /// Sets the values of the voxel at x,y,z and creates it in the sparse voxelgrid.
  /// if setting many voxels at the same time, loop over z first, followed by y,
  /// and from go from HIGH values towards LOW. (minimizes resizing)
  /// @param x Coordinate of the voxel, x cannot be negative.
  /// @param y Coordinate of the voxel, y cannot be negative.
  /// @param z Coordinate of the voxel, z cannot be negative.
  /// @param filled Bool determining if the voxel has been filled or not.
  /// @param a Dummy variable for initial tests of size and performance
  /// @param b Dummy variable for initial tests of size and performance
  /// @see getVoxel
  void setVoxel(GLuint x, GLuint y, GLuint z, bool filled, float a, float b);

  /// @brief Get value of voxel at x,y,z.
  ///
  /// Sets the values of the voxel at x,y,z and creates it in the sparse voxelgrid.
  /// if setting many voxels at the same time, loop over z first, followed by y,
  /// and from go from LOW values towards HIGH. (maximizes cache hits)
  /// @param x Coordinate of the voxel, x cannot be negative.
  /// @param y Coordinate of the voxel, y cannot be negative.
  /// @param z Coordinate of the voxel, z cannot be negative.
  /// @see setVoxel
  /// @return Returns a pointer to the voxel (i.e. changes can be made.) If no voxel exists a nullpointer is returned.
  voxel* getVoxel(GLuint x, GLuint y, GLuint z);


  std::vector<std::array<int, 2>>* LayerFloodFill(int init_x, int init_z, int height);
  void LayerFloodFill_Rec(int x, int z, int height, std::vector<std::array<int, 2>>* filled_coords);
  void FloodFill(int x, int z, int height);


  /// @brief Create a data pointer to all existing voxels
  ///
  /// Goes through all voxels and creates an array containing the positions, used for uploading data to the GPU.
  /// @return Returs a pointer to a vector with all the positions in the order x1,y1,z1,x2,y2,z2 ...
  std::vector<GLuint> *getVoxelPositions();

  /// @brief Initialize Voxelgrid for drawing
  ///
  /// Gets the positions of all currently filled voxels, counts them, creates shader program, VAO and VBO.
  /// @see drawVoxels
  void initDraw();

  /// @brief Updates the buffer on the GPU
  ///
  /// Which can be rendered by using drawVoxels()
  /// @see initDraw
  /// @see drawVoxels
  void updateVoxelrender();

  /// @brief Draw the voxel data
  ///
  /// Draws the current buffer of voxels to the screen as billboards
  /// @see initDraw
  /// @see updateVoxelrender
  void drawVoxels(glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
};

#endif
