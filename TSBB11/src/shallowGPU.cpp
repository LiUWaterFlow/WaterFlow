#include "shallowGPU.h"
#include "GL_utilities.h"
#include "Utilities.h"
#include <algorithm>
#include <stdio.h>
#include <valarray>

#include <iostream>
#include <iomanip>
#include <inttypes.h>


bool DEBUG = false;
	
void ShallowGPU::cycleBuffer(){
	const static GLuint relBuffers[3] = {0,2,4};
	cycle = (cycle +1)%3;
	bufferOut = relBuffers[cycle];
	
}
	
void ShallowGPU::floodFill(float* u, int x, int z,float height){

  std::vector<std::vector<int>> queue;

  if (terr->getData()[x +texWidth*z] < height) {
    queue.push_back({x, z});
  }


  int temp_x, temp_z;

  /* While queue is not empty, keep processing queue from back to front.
  */
  while(queue.size() > 0){

	temp_x  = queue.back().at(0);
	temp_z = queue.back().at(1);

	queue.pop_back();

	// create the four neighbours clamp to ensure we don't check outside the map.	
	int temp_x_plus = clip(temp_x+1,0,texWidth-1);
	int temp_x_min = clip(temp_x-1,0,texWidth-1);
	int temp_z_plus = clip(temp_z+1,0,texHeight-1);
	int temp_z_min = clip(temp_z-1,0,texHeight-1);
	int offset0 = temp_x + temp_z*texWidth; 	
	if(u[offset0] != height && terr->getData()[offset0] < height){
		
		//set height at offset0
		u[offset0] = height;
		//add the four neighbours
		queue.push_back({temp_x_plus,temp_z_min});
		queue.push_back({temp_x_plus,temp_z_plus});		
		queue.push_back({temp_x_min,temp_z_min});		
		queue.push_back({temp_x_min,temp_z_plus});
	}
  }
}
	
int ShallowGPU::clip(int n, int lower, int upper) {
	return std::max(lower, std::min(n, upper));
}

GLfloat ShallowGPU::clipf(GLfloat n, GLfloat lower, GLfloat upper) {
	return std::max(lower, std::min(n, upper));
}


void ShallowGPU::initGPU() {
	if(DEBUG){
	texWidth = 20;
	texHeight = 20;
	}
	float* u = new float[texWidth*texHeight];
	float* vx = new float[texWidth*texHeight];
	float* vy = new float[texWidth*texHeight];
	float* f = new float[texWidth*texHeight];
	float* emp = new float[texWidth*texHeight];
	float* flow = new float[texWidth*texHeight];
	std::fill_n(vx,texWidth*texHeight,0.0f);	
	std::fill_n(vy,texWidth*texHeight,0.0f);	
	std::fill_n(u,texWidth*texHeight,0.0f);	
	std::fill_n(f,texWidth*texHeight,0.0f);	
	std::fill_n(emp,texWidth*texHeight,0.0f);
	std::fill_n(flow, texWidth*texHeight, 0.0f);
	

	int upper2 = 180;
	int lower2 = 70;

	int upper3 = 50;
	int lower3 = 0;

	
	for (int j = 0; j < texHeight; ++j) {
		for (int i = 0; i < texWidth; ++i) {
			u[j*texWidth + i] = 0.0f;
			vx[j*texWidth + i] = 0.0f;
			vy[j*texWidth + i] = 0.0f;
			
			if(i < upper2 && i >lower2 && j < upper2 && j > lower2){
				//add interesting data here
				u[j*texWidth + i] = 60.0f;
			}

			if (i < upper3 && i >lower3 && j < upper3 && j > lower3) {
				//add interesting data here.
				flow[j*texWidth + i] = 0.25f;
			}

		}
	}
	if(DEBUG){
	u[10 + 10 * texWidth] = 5.0f; 
	}

	advectWaterProgram = compileComputeShader("src/shaders/advectWaterShader.comp");
	addProgram = compileComputeShader("src/shaders/addHeightShader.comp");
	advectVelocityXProgram = compileComputeShader("src/shaders/advectVelocityXShader.comp");
	advectVelocityYProgram = compileComputeShader("src/shaders/advectVelocityYShader.comp");
	updateHeightProgram = compileComputeShader("src/shaders/updateHeightShader.comp");
	updateVelocityProgram = compileComputeShader("src/shaders/updateVelocityShader.comp");
	//create buffers
	glGenBuffers(8, shallowBuffers);
	GLint numData = texWidth*texHeight;

	//#warn u is currently total height of terrain, maybe not correct for this impl.

	//Buffer 0, height of water.
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, shallowBuffers[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat)* 1 * numData, u, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	//Buffer 1, height of water pong.
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, shallowBuffers[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat)* 1 * numData, u, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	//Buffer 2, vx 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, shallowBuffers[2]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat)* 1 * numData, vx, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	//Buffer 3 vx pong
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, shallowBuffers[3]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat)* 1 * numData, vx, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
	//Buffer 2, vy
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, shallowBuffers[4]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat)* 1 * numData, vy, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	//Buffer 3 vy pong
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, shallowBuffers[5]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat)* 1 * numData, vy, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	//Buffer 4, terrain data.
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, shallowBuffers[6]);			//use terr->getData() to use terrain
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * 1 * numData,f, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
	//Buffer 8, to renderer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, shallowBuffers[7]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * 1 * numData,emp, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
	//Buffer 9, constant input
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, shallowBuffers[8]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * 1 * numData, flow, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	

	printError("init Compute Error");
	printProgramInfoLog(advectWaterProgram, "AdvectWater", NULL, NULL, NULL, NULL);
	printError("init Compute Error");
	printProgramInfoLog(advectVelocityXProgram, "AdvectX", NULL, NULL, NULL, NULL);
	printError("init Compute Error");
	printProgramInfoLog(advectVelocityYProgram, "AdvectY", NULL, NULL, NULL, NULL);
	printError("init Compute Error");
	printProgramInfoLog(addProgram, "Add", NULL, NULL, NULL, NULL);
	printError("init Compute Error");
	printProgramInfoLog(updateHeightProgram, "Add", NULL, NULL, NULL, NULL);
	printError("init Compute Error");
	printProgramInfoLog(updateVelocityProgram, "Add", NULL, NULL, NULL, NULL);
	
	
	float dt = 0.05f;
	
	glUseProgram(advectWaterProgram);
	glUniform2i(glGetUniformLocation(advectWaterProgram, "size"), texWidth,texHeight);
	glUniform1f(glGetUniformLocation(advectWaterProgram, "dt"), dt);
	
	glUseProgram(advectVelocityXProgram);
	glUniform2i(glGetUniformLocation(advectVelocityXProgram, "size"), texWidth, texHeight);
	glUniform1f(glGetUniformLocation(advectWaterProgram, "dt"), dt);
	
	glUseProgram(advectVelocityYProgram);
	glUniform2i(glGetUniformLocation(advectVelocityYProgram, "size"),texWidth, texHeight);
	glUniform1f(glGetUniformLocation(advectWaterProgram, "dt"), dt);

	glUseProgram(addProgram);
	glUniform2i(glGetUniformLocation(addProgram, "size"),texWidth, texHeight);
	glUniform1f(glGetUniformLocation(advectWaterProgram, "dt"), dt);
	
	glUseProgram(updateHeightProgram);
	glUniform2i(glGetUniformLocation(updateHeightProgram, "size"), texWidth,texHeight);
	glUniform1f(glGetUniformLocation(advectWaterProgram, "dt"), dt);
	
	glUseProgram(updateVelocityProgram);
	glUniform2i(glGetUniformLocation(updateVelocityProgram, "size"), texWidth, texHeight);
	glUniform1f(glGetUniformLocation(advectWaterProgram, "dt"), dt);

	/*
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, shallowBuffers[0]);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLfloat)*texWidth*texHeight, u);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	*/
	//std::valarray<float> myvalarray(u, texWidth*texHeight);
	//vol0 = myvalarray.sum();
	
	delete u;
	delete vx;
	delete vy;
	delete flow;
}

void ShallowGPU::initFloodFill(float* u){
	for(unsigned int i = 0; i < flood.size(); i++){
			floodFill(u, flood.at(i)->x, flood.at(i)->z,flood.at(i)->height);
	}
}



void ShallowGPU::runSimGPU(GLfloat dt) {
	totTime += dt; 
	int numPing = 5;
	if(DEBUG){
		numPing = 500;	
	}

	//Add this one
	glBindBuffersBase(GL_SHADER_STORAGE_BUFFER, 4, 1, &shallowBuffers[0]);
	//and this one.
	glBindBuffersBase(GL_SHADER_STORAGE_BUFFER, 10, 1, &shallowBuffers[8]);
	//write to this one.
	glBindBuffersBase(GL_SHADER_STORAGE_BUFFER, 11, 1, &shallowBuffers[1]);
	glUseProgram(addProgram);
	glDispatchCompute((GLuint)ceil((GLfloat)texWidth / 16.0f), (GLuint)ceil((GLfloat)texHeight / 16.0f), 1);
	glFinish();
	// Initiate ping ponging.
	std::swap(shallowBuffers[0], shallowBuffers[1]);
	for (int i = 0; i < numPing; i++) {

		glBindBuffersBase(GL_SHADER_STORAGE_BUFFER, 4, 7, shallowBuffers);
		
		//select advectWater
		glUseProgram(advectWaterProgram);
	
		glDispatchCompute((GLuint)ceil((GLfloat)texWidth / 16.0f), (GLuint)ceil((GLfloat)texHeight / 16.0f), 1);
		glFinish();
			if(DEBUG){
		
			Print(shallowBuffers[1], "HEIGHT AFTER ADVECT WATER", 1);
			Print(shallowBuffers[2], "VELX AFTER ADVECT WATER", 1);
			Print(shallowBuffers[4], "VELY AFTER ADVECT WATER", 1);
			std::getchar();
			}
		//select velocity advect X
		glUseProgram(advectVelocityXProgram);
		
		glDispatchCompute((GLuint)ceil((GLfloat)texWidth / 16.0f), (GLuint)ceil((GLfloat)texHeight / 16.0f), 1);
		
		glFinish();
		
			if(DEBUG){
			Print(shallowBuffers[1], "HEIGHT AFTER ADVECTVELX", 1);
			Print(shallowBuffers[3], "VELX AFTER ADVECTVELX", 1);
			Print(shallowBuffers[4], "VELY AFTER ADVECTVELX", 1);
			std::getchar();
			}
		//select velocity advect Y
		glUseProgram(advectVelocityYProgram);
		
		glDispatchCompute((GLuint)ceil((GLfloat)texWidth / 16.0f), (GLuint)ceil((GLfloat)texHeight / 16.0f), 1);
		glFinish();
			if(DEBUG){
			Print(shallowBuffers[1], "HEIGHT AFTER ADVECT ADVECTVELY", 1);
			Print(shallowBuffers[3], "VELX AFTER ADVECT ADVECTVELY", 1);
			Print(shallowBuffers[5], "VELY AFTER ADVECT ADVECTVELY", 1);
			std::getchar();
			}
		glUseProgram(updateHeightProgram);
		
		glDispatchCompute((GLuint)ceil((GLfloat)texWidth / 16.0f), (GLuint)ceil((GLfloat)texHeight / 16.0f), 1);
		glFinish();
			if(DEBUG){
			Print(shallowBuffers[0], "HEIGHT AFTER UPDATE HEIGHT", 1);
			Print(shallowBuffers[3], "VELX AFTER UPDATE HEIGHT", 1);
			Print(shallowBuffers[5], "VELY AFTER UPDATE HEIGHT", 1);
			std::getchar();
			}
		//
		
		glUseProgram(updateVelocityProgram);
		
		glDispatchCompute((GLuint)ceil((GLfloat)texWidth / 16.0f), (GLuint)ceil((GLfloat)texHeight / 16.0f), 1);
		glFinish();
			if(DEBUG){
			Print(shallowBuffers[0], "HEIGHT AFTER UPDATE VELOCITY", 1);
			Print(shallowBuffers[2], "VELX AFTER UPDATE VELOCITY", 1);
			Print(shallowBuffers[4], "VELY AFTER UPDATE VELOCITY", 1);
			std::getchar();
		
		
			Print(shallowBuffers[0], "HEIGHT0 AFTER ALL", 1);
			Print(shallowBuffers[2], "VELX0 AFTER ALL", 1);
			Print(shallowBuffers[4], "VELY0 AFTER ALL ", 1);
			std::getchar();
		
		
			Print(shallowBuffers[1], "HEIGHT1 AFTER ALL", 1);
			Print(shallowBuffers[3], "VELX1 AFTER ALL", 1);
			Print(shallowBuffers[5], "VELY1 AFTER ALL ", 1);
		
			}
		
		
		/*
	std::swap(shallowBuffers[0],shallowBuffers[1]);
	std::swap(shallowBuffers[3],shallowBuffers[2]);
	std::swap(shallowBuffers[4],shallowBuffers[5]);
	*/
	}

	//Bind input data(to render)
	glBindBuffersBase(GL_SHADER_STORAGE_BUFFER,4,1,&shallowBuffers[bufferOut]);
	//Bind terrain
	glBindBuffersBase(GL_SHADER_STORAGE_BUFFER,10,1,&shallowBuffers[6]);
	//bind output
	glBindBuffersBase(GL_SHADER_STORAGE_BUFFER,11,1,&shallowBuffers[7]);
	glUseProgram(addProgram);
	glDispatchCompute((GLuint)ceil((GLfloat)texWidth / 16.0f), (GLuint)ceil((GLfloat)texHeight / 16.0f), 1);
	glFinish();
	printError("run Sim GPU");
	
	
}

void ShallowGPU::Print(GLuint bufferID, std::string msg, int iter) const
{
	float * arr = new float[texWidth*texHeight];
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLfloat)*texWidth*texHeight, arr);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	PrintHelper("BEGIN", msg, iter);
	for (int j = 0; j < texHeight; j++)
	{
		for (int i = 0; i < texWidth; i++)
		{

			const unsigned int index = i + j*texWidth;
			PrintNumber(arr[index]);
		}
		std::cout << "\n";
	}
	PrintHelper("END", msg, iter);
	std::cout << std::flush;
}

void ShallowGPU::PrintNumber(float value) const
{
	std::cout << std::fixed << std::setw(7) << std::setprecision(3) << value << " ";
}

void ShallowGPU::PrintHelper(std::string start_end, std::string msg, int iter) const
{
	if (iter != -1) {
		std::cout << "========== " << start_end << " " << msg << " " << iter << " ==========\n\n";
	}
	else {
		std::cout << "========== " << start_end << " " << msg << " ==========\n\n";
	}
}


