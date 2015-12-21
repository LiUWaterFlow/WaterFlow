#ifndef VOXEL_H
#define VOXEL_H

#include <vector>
#include <inttypes.h>

#include "glm.hpp"
#include "readData.h"
#include "glm.hpp"

/// @struct voxel
/// @brief Contains information and data in each voxel.
///

struct voxel{
  bool filled;  ///<The fill state of the voxel (maybe not needed)
  float a;      ///<Float a (temporary for space measurements and testing)
  float b;      ///<Float b (temporary for space measurements and testing)
  int16_t x;    ///< x position of the voxel
  int16_t y;    ///< y position of the voxel
  int16_t z;    ///< z position of the voxel
};

/// @struct neighs
/// @brief Struct used for returning a voxel and it's neighbours
///
/// The voxels that get returned are ordered with the input voxel as the center voxel
/// each coordinate loops from -1 to plus 1. x is the outermost coordinate, y in the middle and
/// z the innermost variable. example voxs[5] (i.e the sixth element) will be
/// 6-3 = 3  (increase y to 0) 3-3 = 0, three steps in z, gives z = 1;
/// x = -1, y = 0, z = +1; in offsets from the center pixel.
/// @see xoff
/// @see yoff
/// @see zoff
/// @see getNeighbourhood
struct neighs {
	voxel* voxs[27]; ///< array of the voxels in the neighbourhood
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

  std::vector<GLuint>* voxelPositions; ///<Vector for the position where there are voxels, for drawing purposes.
  GLuint numVoxels; ///< Number of voxels in the voxelPositions.
  GLuint voxelShader; ///< Shader program.
  GLuint voxelBuffer;///< Buffer for rendering
  GLuint voxelVAO; ///< VAOs for rendering

  std::vector<GLint>* waterHeight; ///< The map of height data for the topmost water voxel.
  GLuint width; ///< The width of the map, needed for saving data in linear containers.
  GLuint height; ///< The height of the map, needed for voxel extraction


  GLfloat rehashTresh; ///< Rehash treshhold.


  std::vector<voxel*>* hashTable; ///< Pointer to hashTable. Filled with voxel pointers.


  ///@brief hashFunc which returns a position in the hashtable to insert the voxel
  ///
  /// The hashfunction multiplies each coordninate with a large prime.
  /// then takes xor between the results, takes the absolute value and finally
  /// returns the value modulus the hashtable size.
  int64_t hashFunc(int64_t x, int64_t y, int64_t z,int64_t inHashSize);

  int xoff[27]; ///< xoffsets for neighbour extraction.
  int yoff[27]; ///< xoffsets for neighbour extraction.
  int zoff[27]; ///< xoffsets for neighbour extraction.


public:

  int64_t hashSize; ///< size of the hash table

  GLuint numInTable; ///< How many elements there are in the hashTable.
  GLuint numCollisions; ///< How many collisions presently

  /// @brief Constructs an empty voxel grid
  ///
  /// Constructs an initially empty sparse voxelgrid, which scales so that there
  /// is N number of voxels in height representing the lowest point to the heighest.
  /// Where N is ceil(DataHandler->getTerrainScale)
  /// The grid dynamically grows when voxels are added.
  /// The grid supports values between -10 and INT_MAX-10, by offsetting input coordinates with +10.
  /// @param handle Handle to the DataHandler will be bound to the class
  /// @param hashSize intialsize of the hashTable. Set if you plan on using the hashfunctions,
  /// should be a power of 2 for quick calculations.
  /// @see DataHandle::getTerrainScale
  /// @see setVoxel
  /// @see getVoxel
  /// @todo The Voxelgrid should be able to handle one voxel outside the area of the terrain in each dimension.
  Voxelgrid(DataHandler* handle,int64_t hashSize);


  /// @brief Completely delete the sparse voxelgrid.
  ~Voxelgrid();

  /// @brief Set value of voxel at x,y,z.
  ///
  /// Sets the values of the voxel at x,y,z and creates it in the sparse voxelgrid.
  /// if setting many voxels at the same time, loop over z first, followed by y,
  /// and from go from HIGH values towards LOW. (minimizes resizing)
  /// negative values supported by offsetting input by 10.
  /// supports values in the range -10 to INT_MAX-10
  /// @param x Coordinate of the voxel, x cannot be smaller than -10
  /// @param y Coordinate of the voxel, y cannot be smaller than -10
  /// @param z Coordinate of the voxel, z cannot be smaller than -10
  /// @param filled Bool determining if the voxel has been filled or not.
  /// @param a Dummy variable for initial tests of size and performance
  /// @param b Dummy variable for initial tests of size and performance
  /// @see getVoxel
  void setVoxel(int16_t x, int16_t y, int16_t z, bool filled, float a, float b);

  /// @brief Get value of voxel at x,y,z.
  ///
  /// Sets the values of the voxel at x,y,z and creates it in the sparse voxelgrid.
  /// if setting many voxels at the same time, loop over z first, followed by y,
  /// and from go from LOW values towards HIGH. (maximizes cache hits)
  /// @param x Coordinate of the voxel, x cannot be smaller than -10
  /// @param y Coordinate of the voxel, y cannot be smaller than -10
  /// @param z Coordinate of the voxel, z cannot be smaller than -10
  /// @see setVoxel
  /// @return Returns a pointer to the voxel (i.e. changes can be made.) If no voxel exists a nullpointer is returned.
  voxel* getVoxel(int16_t x, int16_t y, int16_t z);

  ///@brief Floodfills pixels at the specified position and height.
  ///
  /// Floodfills at height, stopping when hitting terrain which is heigher than
  /// the specified flood height. Currently only fills voxels at the topmost layer
  /// for optimization reasons.
  void FloodFill(int x, int z, int height, bool fillDown = true);


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

  ///@brief Adds a pixel using the hash functions.
  ///
  ///@warning Please use setVoxel instead since it for most cases is much faster.
  void hashAdd(int16_t x, int16_t y, int16_t z,bool filled, float a, float b);
  ///@brief Gets a pixel using the hash functions.
  ///
  ///@see setVoxel
  ///@warning Please use the getVoxel instead, since it for most cases is much faster.
  voxel* hashGet(int16_t x, int16_t y, int16_t z);
  ///@brief rehashes the table.
  ///
  ///@see getVoxel
  ///@warning This operation is potentially very slow. Please use a sufficiently
  /// large table from the start instead or use the setVoxel and getVoxel instead.
  void rehash();

  ///@brief Returns true if the voxel contains the coordinates specified by x,y,z
  ///
  bool isEqualPoint(voxel* vox,short int x, short int y,short int z);
  ///@brief Initializes the hashmap to the size specified by hashSize.
  ///
  ///@warning Use voxelgrid instead due to speed.
  ///@see setVoxel
  ///@see getVoxel
  void hashInit();

  ///@brief Get a voxel and the 26 neighbouring pixels.
  ///
  ///@see neighs
  neighs* getNeighbourhood(int16_t x, int16_t y, int16_t z);

  ///@brief Get a voxel and the 26 neighbouring pixels.
  ///
  ///@warning Use getNeighbourhood instead.
  ///@see getNeighbourhood
  neighs* getNeighbourhoodHash(int16_t x, int16_t y, int16_t z);


  /// @brief Sets the height in waterHeight.
  ///
  /// The function only sets the data granted that height is larger than the value currently
  /// residing there.
  /// @see getHeight
  void setHeight(int16_t x, int16_t y, int16_t z);

  /// @brief Gets the height in waterHeight
  ///
  /// The function returns the value at x,z in waterHeight
  /// @see getHeight
  GLint getHeight(int16_t x, int16_t z);

  /// @brief Returns a pointer to waterHeight.
  ///
  /// @warning The heightMap is NOT updated on deletion of a voxel only on addition.
  /// @todo The heightMap SHOULD be updated on deletion of a voxel.
  /// @see getHeight
  /// @see setHeight
  /// @see waterHeight
  std::vector<GLint>* getHeightMap();

};

#endif
