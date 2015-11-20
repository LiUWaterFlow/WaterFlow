#include "heightField.h"
#include "GL_utilities.h"
#include <algorithm>
#include <stdio.h>

void HeightField::initTest(){
  for (size_t i = 0; i < width; i++) {
    for (size_t j = 0; j < height; j++) {
      u[i][j] =terr->giveHeight(i*samp,j*samp)-2;
      v[i][j] = 0;
    }
  }
  for (int j = 20; j < 30 ; j++){
	for (int i = 20; i <30 ; i++) {
	  u[i][j] = u[i][j] + 5;
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
		if (u[i][j] - terr->giveHeight(i*samp, j*samp) < 0.0f) {
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


void HeightField::initGPU(){

	float* u = new float[texWidth*texHeight];
	float* v = new float[texWidth*texHeight];

	int upper = 505;
	int lower = 30; 

	for (int j = 0; j < texHeight; ++j){
		for(int i = 0; i < texWidth; ++i){
			u[j*texWidth+i] = terr->getCoord(i,j);
			v[j*texWidth+i] = 0.0f;	

			//CREATE INTERESTING DATA HERE.			
			if(i > lower && i < upper && j > lower && j < upper) {
				u[j*texWidth +i] += 0.05;			
			}
		}
		
	}

	int i = 0;
	  
	fieldProgram = glCreateProgram();	
  
	fieldShader = glCreateShader(GL_COMPUTE_SHADER);
  
	const char* cs = readFile((char *)"src/shaders/fieldShader.comp");
  

	if (cs == NULL){
		printf("Error reading shader \n");
	}

	glShaderSource(fieldShader,1,&cs,NULL);
  
	glCompileShader(fieldShader);
	
	  
	//get errors 
	printError("init Compute Error 1" );
	printShaderInfoLog(fieldShader, "init Compute Error 1");	
	glAttachShader(fieldProgram,fieldShader);
	glLinkProgram(fieldProgram);
	//get errors from the program linking.
	printError("init Compute Error 2" );
	printShaderInfoLog(fieldShader, "init Compute Error 2");
	
	  
	//create buffers
	glGenBuffers(6,fieldBuffers);
	GLint numData = terr->getDataWidth()*terr->getDataHeight();

	glBindBuffer(GL_SHADER_STORAGE_BUFFER,fieldBuffers[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER,sizeof(GLfloat)*1*numData,u,GL_STATIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER,fieldBuffers[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER,sizeof(GLfloat)*1*numData,NULL,GL_STATIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER,fieldBuffers[2]);
	glBufferData(GL_SHADER_STORAGE_BUFFER,sizeof(GLfloat)*1*numData,v,GL_STATIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER,fieldBuffers[3]);
	glBufferData(GL_SHADER_STORAGE_BUFFER,sizeof(GLfloat)*1*numData,terr->getData(),GL_STATIC_DRAW);
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER,fieldBuffers[4]); //vertex data
	glBufferData(GL_SHADER_STORAGE_BUFFER,sizeof(GLfloat)*3*numData,NULL,GL_STATIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER,fieldBuffers[5]); //indexes
	GLuint numIndices = (terr->getDataWidth()-2)*(terr->getDataHeight()-2)*2*3;
	glBufferData(GL_SHADER_STORAGE_BUFFER,sizeof(GLfloat)*numIndices,NULL,GL_STATIC_DRAW);
	
	drawBuffers[0] = fieldBuffers[4]; //vertex 
	drawBuffers[1] = 0; //tex
	drawBuffers[2] = fieldBuffers[5]; //indices
	drawBuffers[3] = 0; //normals
	printError("init Compute Error" );
	printProgramInfoLog(fieldProgram, "field Init", NULL,NULL,NULL,NULL);

	delete u; 
	delete v;

}

void HeightField::bindSimGPU(){
	glBindBuffersBase(GL_SHADER_STORAGE_BUFFER,0,1,&fieldBuffers[4]);
	glBindBuffersBase(GL_SHADER_STORAGE_BUFFER,2,1,&fieldBuffers[5]);
	glBindBuffersBase(GL_SHADER_STORAGE_BUFFER,3,1,&fieldBuffers[0]);
	terr->runCompute();
		
}


void HeightField::runSimGPU(){

	int numPing = 10;

	glUseProgram(fieldProgram);	
	printError("run Compute Error UseProg" );

	glUniform2i(glGetUniformLocation(fieldProgram,"size"),terr->getDataWidth(),terr->getDataHeight());
	printError("run Compute Error 3" );

	//here we will do it a bit different since the first two buffers will ping-pong.	

	glBindBuffersBase(GL_SHADER_STORAGE_BUFFER,4,4,fieldBuffers);
	
	for(int i = 0; i < numPing ; i++){
	printError("run Compute Error 4" );
	glDispatchCompute((GLuint)ceil(terr->getDataWidth()/16),(GLuint)ceil(terr->getDataHeight()/16),1);
	printError("run Compute Error 5" );
	std::swap(fieldBuffers[0],fieldBuffers[1]);
	glBindBuffersBase(GL_SHADER_STORAGE_BUFFER,4,4,fieldBuffers);
	}
	bindSimGPU();
	
}






