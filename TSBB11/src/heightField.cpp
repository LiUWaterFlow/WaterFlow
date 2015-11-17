#include "heightField.h"
#include "GL_utilities.h"

void HeightField::initTest(){
  for (size_t i = 1; i < 99; i++) {
    for (size_t j = 1; j < 99; j++) {
      u[i][j] = 0;
      v[i][j] = 0;
    }
  }

  u[50][50] = 0.3;
}

void HeightField::updateSim(){
  GLfloat c2 = 0.99;
  GLfloat h2 = 4;
  GLfloat dt = 1.0f/60.0f;
  for (size_t i = 1; i < 99; i++) {
    for (size_t j = 1; j < 99; j++) {
      GLfloat f = c2 * (u[i-1][j]+u[i-1][j]+u[i][j-1]+u[i][j+1]-4*u[i][j])/h2;
      v[i][j] = v[i][j] + f*dt;
      unew[i][j] = u[i][j]+ v[i][j]*dt;
    }
  }

  for (size_t i = 1; i < 99; i++) {
    for (size_t j = 1; j < 99; j++) {
      u[i][j] = unew[i][j];
    }
  }
}

void HeightField::render(){
  updateVoxelrender();
}

std::vector<GLuint> *HeightField::getVoxelPositions() {
  std::vector<GLuint> *positions = new std::vector<GLuint>;

  for (size_t i = 0; i < 100; i++) {
    for (size_t j = 0; j < 100; j++) {
      positions->push_back(i);
      positions->push_back(u[i][j]);
      positions->push_back(j);
    }
  }
  return positions;
}

void HeightField::initDraw() {
  voxelPositions = getVoxelPositions();
  numVoxels = voxelPositions->size() / 3;
  voxelShader = loadShadersG("src/shaders/simplevoxels.vert", "src/shaders/simplevoxels.frag", "src/shaders/simplevoxels.geom");

  glGenBuffers(1, &voxelBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, voxelBuffer);
  glBufferData(GL_ARRAY_BUFFER, numVoxels * 3 * sizeof(GLuint), voxelPositions->data(), GL_STATIC_COPY);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ARRAY_BUFFER, voxelBuffer);
  glGenVertexArrays(1, &voxelVAO);
  glBindVertexArray(voxelVAO);

  GLuint posAttrib = glGetAttribLocation(voxelShader, "posValue");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 3, GL_UNSIGNED_INT, GL_FALSE, 0, 0);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void HeightField::updateVoxelrender() {
  voxelPositions = getVoxelPositions();
  numVoxels = voxelPositions->size() / 3;

  glBindBuffer(GL_ARRAY_BUFFER, voxelBuffer);
  glBufferData(GL_ARRAY_BUFFER, numVoxels * 3 * sizeof(GLuint), voxelPositions->data(), GL_STATIC_COPY);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void HeightField::drawVoxels(glm::mat4 projectionMatrix, glm::mat4 viewMatrix) {
  glUseProgram(voxelShader);
  glBindBuffer(GL_ARRAY_BUFFER, voxelBuffer);
  glBindVertexArray(voxelVAO);
  glUniformMatrix4fv(glGetUniformLocation(voxelShader, "VTPMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
  glUniformMatrix4fv(glGetUniformLocation(voxelShader, "WTVMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));

  if (numVoxels > 0) {
    glDrawArrays(GL_POINTS, 0, numVoxels);
  }

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  printError("Voxel Draw Billboards");
}
