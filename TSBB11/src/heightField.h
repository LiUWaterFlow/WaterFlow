#ifndef HEIGHTFIELD_H
#define HEIGHTFIELD_H
#include "readData.h"
#include "glm.hpp"
#include "gtc/type_ptr.hpp"
#include <vector>

class HeightField{
private:

  GLuint numVoxels; ///< Number of voxels in the voxelPositions.
  GLuint voxelShader; ///< Shader program.
  GLuint voxelBuffer, voxelVAO; ///< Buffers and VAOs.

  DataHandler* dataHandler;
  GLfloat u[100][100];
  GLfloat unew[100][100];
  GLfloat v[100][100];
  int width;
  int height;
  std::vector<GLuint>* voxelPositions;


public:
  HeightField(){};
  void updateSim();
  void render();
  void initDraw();
  void initTest();
  void updateVoxelrender();
  void drawVoxels(glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
  std::vector<GLuint>* getVoxelPositions();
};



#endif
