/// @file shallowGPU.h
/// @brief Contains the CPU and GPU implementation of heightfield.


#ifndef SHALLOWGPU_H
#define SHALLOWGPU_H
#include "readData.h"
#include "glm.hpp"
#include "gtc/type_ptr.hpp"
#include <vector>
#include "xmlParsing.h"

/// @class ShallowGPU
/// @brief Handles the shallow water testcase simulation.
class ShallowGPU{
private:
  GLint texWidth; ///< width of simulation area
  GLint texHeight; ///< height of simulation area

  float* flow; ///< Array of added flow per iteration

  DataHandler* terr; ///< Handle to terrain
  GLuint cycle; ///< Cycle used for selecting render output

  GLuint bufferOut; ///< Out buffer to be rendered.

  /// @brief binds the necessary buffers for rendering (CPU)
  ///
  /// This should be removed
  void bindSimGPU();
  /// @brief clamps the int value
  ///
  /// Clamps the int value n, between lower and upper.
  int clip(int n, int lower, int upper);

  /// @brief clamps the float value
  ///
  /// Clamps the float value n, between lower and upper.
  float clipf(GLfloat n, GLfloat lower, GLfloat upper);


  GLfloat totTime; ///< Variable for time measurements

  std::vector<Flood_Fill_data*> flood; ///< Floodfill data structure


  /// @brief Runs the floodfill algorithms
  ///
  /// Data is provided for floodpoints at construction.
  void initFloodFill(float* u);

  /// @brief Floodfills point x,z to height and saves in heightmap u.
  ///
  /// Data is provided for floodpoints at construction.
  void floodFill(float* u, int x, int z, float height);

  bool firstTime; ///< First time check (can be removed)

  /// @brief Prints debug info for buffer at bID.
  ///
  /// Prints the message and the iteration aswell.
  void Print(GLuint bID, std::string msg, int iter) const;

  /// @brief helper function for print.
  ///
  void PrintNumber(float value) const;

  /// @brief helper function for print.
  ///
  void PrintHelper(std::string start_end, std::string msg, int iter) const;

public:

  /// @brief change output buffer
  ///
  void cycleBuffer();


  GLuint drawBuffers[4]; ///< drawBuffers (can be removed)
  GLuint advectWaterProgram ///< shader used for simulation
  GLuint addProgram;///< shader used for simulation
  GLuint advectVelocityXProgram;///< shader used for simulation
  GLuint advectVelocityYProgram;///< shader used for simulation
  GLuint updateHeightProgram; ///< shader used for simulation
  GLuint updateVelocityProgram;///< shader used for simulation
  GLuint shallowBuffers[9];///< Buffers used for passing data between shaders

  /// @brief Constructor for shallowGPU
  ///
  /// Must take handle to terrain and a floodfill data vector
  ShallowGPU(DataHandler *t,std::vector<Flood_Fill_data*> FFDataIn) { terr = t; texWidth = t->getDataWidth();
  texHeight = t->getDataHeight(); totTime = 0.0f; flood = FFDataIn; firstTime = true; bufferOut = 0; cycle = 0;};

  /// @brief runs the simulation one timestep (CPU)
  ///
  /// This should be removed
  void updateSim(GLfloat);

  /// @brief Initializes GPU for simulation
  ///
  void initGPU();

  /// @brief Runs the simulation one timestep.
  ///
  ///
  void runSimGPU(GLfloat dt = 1.0f/30.0f);
};



#endif
