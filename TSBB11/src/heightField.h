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

  const static int width = 1000;
  const static int height = 1000;

  DataHandler* terr;
  GLfloat u[width][height];
  GLfloat unew[width][height];
  GLfloat v[width][height];
  std::vector<GLuint>* voxelPositions;
  GLint samp = 1; 


public:
	HeightField(DataHandler *t) { terr = t; };
  void updateSim();
  void render();
  void initDraw();
  void initTest();
  void updateVoxelrender();
  void drawVoxels(glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
  std::vector<GLuint>* getVoxelPositions();
  GLfloat getHeight(int i ,int j, GLfloat ourHeight);
};



#endif
