#ifndef SHALLOWGPU_H
#define SHALLOWGPU_H
#include "readData.h"
#include "glm.hpp"
#include "gtc/type_ptr.hpp"
#include <vector>
#include "xmlParsing.h"

class ShallowGPU{
private:
  GLint texWidth, texHeight;
  
  float* flow;

  DataHandler* terr;
  GLuint cycle;
  
  GLuint bufferOut;
  void bindSimGPU();
  int clip(int n, int lower, int upper);
  float clipf(GLfloat n, GLfloat lower, GLfloat upper);
  GLfloat totTime;
  std::vector<Flood_Fill_data*> flood;
  void initFloodFill(float* u);
  void floodFill(float* u, int x, int z, float height);
  bool firstTime;
  void Print(GLuint bID, std::string msg, int iter) const;
  void PrintNumber(float value) const;
  void PrintHelper(std::string start_end, std::string msg, int iter) const;

public:

  void cycleBuffer();
  
  GLuint drawBuffers[4];
  GLuint advectWaterProgram,addProgram,advectVelocityXProgram,advectVelocityYProgram,updateHeightProgram,updateVelocityProgram;
  GLuint shallowBuffers[9];
  
  ShallowGPU(DataHandler *t,std::vector<Flood_Fill_data*> FFDataIn) { terr = t; texWidth = t->getDataWidth(); 
  texHeight = t->getDataHeight(); totTime = 0.0f; flood = FFDataIn; firstTime = true; bufferOut = 0; cycle = 0;};
  void updateSim(GLfloat);

  void initGPU();
  void runSimGPU(GLfloat dt = 1.0f/30.0f);
};



#endif
