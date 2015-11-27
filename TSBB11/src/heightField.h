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

  const static int width = 50;
  const static int height = 50;

  GLuint texWidth, texHeight;

  
  GLuint terrTex, heightTex, speedTex;

  DataHandler* terr;
  GLfloat u[width][height];
  GLfloat unew[width][height];
  GLfloat v[width][height];
  std::vector<GLuint>* voxelPositions;
  const static GLint samp = 1;
  void bindSimGPU();
  int clip(int n, int lower, int upper);
  float clipf(GLfloat n, GLfloat lower, GLfloat upper);
  unsigned long long vol0;



public:
  
  GLuint drawBuffers[4];
  GLuint fieldProgram, fieldShader,fieldVAO,addProgram,addShader;
  GLuint fieldBuffers[8];
  
  void floodFill(float* u, int x, int z, float height);

  HeightField(DataHandler *t) { terr = t; texWidth = t->getDataWidth(); texHeight = t->getDataHeight();};
  void updateSim(GLfloat);
  void setTerrainTex(GLuint t){terrTex = t;};
  GLuint getHeightTex(){return heightTex;};
  GLuint getSpeedTex(){return speedTex;};
  void render();
  void initDraw();
  void initTest();
  void initGPU();
  void runSimGPU();
  void updateVoxelrender();
  void drawVoxels(glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
  std::vector<GLuint>* getVoxelPositions();
  GLfloat getHeight(int i ,int j, GLfloat ourHeight);
};



#endif
