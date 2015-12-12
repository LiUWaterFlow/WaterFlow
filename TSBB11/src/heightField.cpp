/// @file heightField.cpp
/// @brief Implementations of functions in heightfield.h

#include "heightField.h"
#include "GL_utilities.h"
#include "Utilities.h"
#include <algorithm>
#include <stdio.h>
#include <valarray>
#include <inttypes.h>

HeightField::HeightField(DataHandler *t, std::vector<Flood_Fill_data*> FFDataIn, std::vector<FlowSource*> FlowsourcesIN) {
	terr = t;
	texWidth = t->getDataWidth();
	texHeight = t->getDataHeight();
	totTime = 0.0f; 
	flood = FFDataIn;
	xmlFlow = FlowsourcesIN;

}

void HeightField::floodFill(float* u, int x, int z, float height) {

	std::vector<std::vector<int>> queue;

	if (terr->getData()[x + texWidth*z] < height) {
		queue.push_back({ x, z });
	}


	int temp_x, temp_z;

	/* While queue is not empty, keep processing queue from back to front.
	*/
	while (queue.size() > 0) {

		temp_x = queue.back().at(0);
		temp_z = queue.back().at(1);

		queue.pop_back();

		// create the four neighbours clamp to ensure we don't check outside the map.	
		int temp_x_plus = clip(temp_x + 1, 0, texWidth - 1);
		int temp_x_min = clip(temp_x - 1, 0, texWidth - 1);
		int temp_z_plus = clip(temp_z + 1, 0, texHeight - 1);
		int temp_z_min = clip(temp_z - 1, 0, texHeight - 1);
		int offset0 = temp_x + temp_z*texWidth;
		if (u[offset0] != height && terr->getData()[offset0] < height) {

			//set height at offset0
			u[offset0] = height;
			//add the four neighbours
			queue.push_back({ temp_x_plus,temp_z_min });
			queue.push_back({ temp_x_plus,temp_z_plus });
			queue.push_back({ temp_x_min,temp_z_min });
			queue.push_back({ temp_x_min,temp_z_plus });
		}
	}
}



void HeightField::initTest() {
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			u[i][j] = terr->giveHeight(i*samp, j*samp) - 2;
			v[i][j] = 0;
		}
	}
	for (int j = 20; j < 30; j++) {
		for (int i = 20; i < 30; i++) {
			u[i][j] = u[i][j] + 5;
		}
	}
}

int HeightField::clip(int n, int lower, int upper) {
	return std::max(lower, std::min(n, upper));
}

GLfloat HeightField::clipf(GLfloat n, GLfloat lower, GLfloat upper) {
	return std::max(lower, std::min(n, upper));
}

GLfloat HeightField::getHeight(int i, int j, GLfloat ourHeight) {
	i = clip(i, 0, width - 1);
	j = clip(j, 0, height - 1);
	if (u[i][j] - terr->giveHeight(i*samp, j*samp) < 0.0f) {
		return ourHeight;
	}
	return u[i][j];

}

void HeightField::updateSim(GLfloat dt) {

	GLfloat c2 = 1;
	float max_c = (1.0 / dt);
	if (c2 > max_c) {
		printf("C2 too large/ dt too large.");
	}

	GLfloat h2 = 4;
	dt = dt*4.9;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {

			GLfloat height_east = getHeight((i + 1), j, u[i][j]);
			GLfloat height_west = getHeight((i - 1), j, u[i][j]);
			GLfloat height_south = getHeight(i, (j - 1), u[i][j]);
			GLfloat height_north = getHeight(i, (j + 1), u[i][j]);

			GLfloat f = c2*(height_west + height_east + height_south + height_north - 4 * u[i][j]) / h2;

			f = clipf(f, -0.1, 0.1);
			v[i][j] = v[i][j] + f*dt;
			unew[i][j] = u[i][j] + v[i][j] * dt;
			v[i][j] *= 0.995;

		}
	}


	memcpy(u, unew, sizeof(unew));
}

void HeightField::render() {
	updateVoxelrender();
}

std::vector<GLuint> *HeightField::getVoxelPositions() {
	std::vector<GLuint> *positions = new std::vector<GLuint>;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			if (u[i][j] - 1 > terr->giveHeight(i, j)) {
				positions->push_back(i*samp);
				positions->push_back(round(u[i][j] - 1));
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


void HeightField::initGPU() {

	float* u = new float[texWidth*texHeight];
	float* v = new float[texWidth*texHeight];
	float* f = new float[texWidth*texHeight];
	std::fill_n(u, texWidth*texHeight, 0.0f);
	std::fill_n(v, texWidth*texHeight, 0.0f);
	std::fill_n(f, texWidth*texHeight, 0.0f);

	initFloodFill(u);

	std::valarray<float> fvalarray(terr->getData(), texWidth*texHeight);
	fvalarray = fvalarray - 0.0001f;
	
	fieldProgram = compileComputeShader("src/shaders/fieldShader.comp"); 
	addProgram = compileComputeShader("src/shaders/addFlowShader.comp");

	//create buffers
	glGenBuffers(5, fieldBuffers);
	GLint numData = terr->getDataWidth()*terr->getDataHeight();

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, fieldBuffers[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * 1 * numData, u, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, fieldBuffers[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * 1 * numData, u, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, fieldBuffers[2]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * 1 * numData, v, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, fieldBuffers[3]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * 1 * numData, &fvalarray[0], GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, fieldBuffers[4]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * 1 * numData, f, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	printError("init Compute Error");
	printProgramInfoLog(fieldProgram, "field Init", NULL, NULL, NULL, NULL);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, fieldBuffers[0]);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLfloat)*texWidth*texHeight, u);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	delete u;
	delete v;
	delete f;

}

void HeightField::initFloodFill(float* u) {
	for (unsigned int i = 0; i < flood.size(); i++) {
		floodFill(u, flood.at(i)->x, flood.at(i)->z, flood.at(i)->height);
	}
}

void HeightField::runSimGPU(GLfloat dt) {
	totTime += dt;
	int numPing = 20;
	GLint numData = terr->getDataWidth()*terr->getDataHeight();

	if (flowChange(xmlFlow, dt)) {
		GLfloat* f = new float[texWidth*texHeight];
		std::fill_n(f, texWidth*texHeight, 0.0f);
		for (unsigned int i = 0; i < xmlFlow.size(); i++) {
			std::vector<int> flowPos = xmlFlow.at(i)->getPosition();
			f[flowPos[0] + flowPos[2] * texWidth] = xmlFlow.at(i)->currPres;
			printf("Pressure %f \n", xmlFlow.at(i)->currPres);
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, fieldBuffers[4]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * 1 * numData, f, GL_STATIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		delete f;
	}


	glUseProgram(fieldProgram);
	glUniform2i(glGetUniformLocation(fieldProgram, "size"), terr->getDataWidth(), terr->getDataHeight());
	glUniform1f(glGetUniformLocation(fieldProgram, "dt"), dt);

	glUseProgram(addProgram);
	glUniform2i(glGetUniformLocation(addProgram, "size"), terr->getDataWidth(), terr->getDataHeight());

	glBindBuffersBase(GL_SHADER_STORAGE_BUFFER, 4, 5, fieldBuffers);

	glUseProgram(addProgram);
	glDispatchCompute((GLuint)ceil((GLfloat)terr->getDataWidth() / 16.0f), (GLuint)ceil((GLfloat)terr->getDataHeight() / 16.0f), 1);


	for (int i = 0; i < numPing; i++) {

		glBindBuffersBase(GL_SHADER_STORAGE_BUFFER, 4, 5, fieldBuffers);


		glUseProgram(fieldProgram);
		glDispatchCompute((GLuint)ceil((GLfloat)terr->getDataWidth() / 16.0f), (GLuint)ceil((GLfloat)terr->getDataHeight() / 16.0f), 1);
		std::swap(fieldBuffers[0], fieldBuffers[1]);
	}

	printError("run Sim GPU");
}


void HeightField::measureVolume() {

	float* u = new float[texWidth*texHeight];

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, fieldBuffers[0]);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, texWidth*texHeight*sizeof(float), u);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	std::valarray<float> myvalarray(u, texWidth*texHeight);
	float vol1 = myvalarray.sum();


	std::cout << "Volume difference is: " << vol1 - vol0 << std::endl;
	myvalarray.resize(0);
	delete u;

}











