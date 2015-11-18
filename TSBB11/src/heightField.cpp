#include "heightField.h"
#include "GL_utilities.h"
#include <algorithm>

void HeightField::initTest(){
  for (size_t i = 0; i < width; i++) {
    for (size_t j = 0; j < height; j++) {
      u[i][j] =terr->giveHeight(i*samp,j*samp)-1;
      v[i][j] = 0;
    }
  }
  for (int j = 25; j < 75; j++){
	for (int i = 25; i <75 ; i++) {
	  u[i][j] = u[i][j] + 25;
	}
  }
}

int clip(int n, int lower, int upper) {
	return std::max(lower, std::min(n, upper));
}

GLfloat clipf(GLfloat n, GLfloat lower, GLfloat upper) {
	return std::max(lower, std::min(n, upper));
}

GLfloat HeightField::getHeight(int i, int j,GLfloat ourHeight) {
		i = clip(i, 0, width-1);
		j = clip(j, 0, height-1);
		if (abs(u[i][j] - terr->giveHeight(i*samp, j*samp)) < 1.0f) {
			return ourHeight; 
		}
		return u[i][j];
	
}

void HeightField::updateSim(GLfloat dt){

	GLfloat c2 = 1;
	float max_c = (1.0 / dt);
	if (c2 > max_c) {
		printf("C2 too large/ dt too large.");
	}

	GLfloat h2 = 4;
	dt = dt*4.9;
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {

		GLfloat height_east = getHeight((i+1) , j, u[i][j]);// u[std::min(i + 1, width)][j];
		GLfloat height_west = getHeight((i-1), j, u[i][j]);// u[std::max(i-1,0)][j];
		GLfloat height_south = getHeight(i, (j-1), u[i][j]);// u[i][std::max(j -1,0)];
		GLfloat height_north = getHeight(i, (j+1), u[i][j]);// u[i][std::min(j + 1, height)];
		
		GLfloat f = c2*(height_west + height_east + height_south + height_north - 4 * u[i][j]) / h2;
		
		f = clipf(f, -0.1, 0.1);
		v[i][j] = v[i][j] + f*dt;
		unew[i][j] = u[i][j] + v[i][j] * dt;
		v[i][j] *= 0.995;
		
		
		//unew[i][j] = u[i][j] + ((height_west + height_east + height_south + height_north) / 4 - u[i][j]) * c2;
	
	}
  }

  
  memcpy(u, unew, sizeof(unew));
}

void HeightField::render(){
  updateVoxelrender();
}

std::vector<GLuint> *HeightField::getVoxelPositions() {
  std::vector<GLuint> *positions = new std::vector<GLuint>;

  for (size_t i = 0; i < width; i++) {
    for (size_t j = 0; j < height; j++) {
		if (u[i][j]-1 > terr->giveHeight(i,j) ){
      positions->push_back(i*samp);
      positions->push_back(round(u[i][j]-1));
      positions->push_back(j*samp);
		}
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
	voxelPositions->clear();
	delete voxelPositions;
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
