/// @file readData.cpp
/// @brief Implementations of functions in readData.h
#include <chrono>
#include "readData.h"

#include "Utilities.h"
#include "GL_utilities.h"

#include "glm.hpp"

#include <iostream>

// ===== Constructors and destructors

DataHandler::DataHandler(const char* inputfile, int sampleFactor, int blockSize)
: sampleFactor(sampleFactor), blockSize(blockSize) {
	readdata = new mapdata();

	std::cout << "Reading DEM data from: " << inputfile << "...";
	readDEM(inputfile);
	std::cout << " done!" << std::endl;

	std::cout << "Performing normalized convolution...";
	normConvCompute();
	std::cout << " done!" << std::endl;
}

DataHandler::~DataHandler() {
	delete readdata;
}

// ===== Getters =====

float DataHandler::getCoord(int col, int row) {
	int index;
	float retdata = 0;

	if (readdata != NULL) {

		if (col < readdata->ncols && row < readdata->nrows) {
			index = col + row * readdata->ncols;
		} else {
			std::cerr << "Input does not exist in data. Col: " << col << " Row: " << row << std::endl;
			index = 0;
		}
		retdata = readdata->data[index];
	} else {
		std::cerr << "No mapdata exists." << std::endl;
	}

	return retdata;
}
float* DataHandler::getData() {
	return &readdata->data[0];
}
int DataHandler::getDataWidth() {
	return readdata->ncols;
}
int DataHandler::getDataHeight() {
	return readdata->nrows;
}
int DataHandler::getElem() {
	return readdata->nelem;
}
GLfloat DataHandler::getTerrainScale() {
	return terrainScale;
}
GLuint DataHandler::getHeightBuffer() {
	return terrainBufferID;
}

// ===== Actual functions =====

void DataHandler::readDEM(const char* inputfile) {
	char* buffer = readFile(inputfile);
	char* currentStr = buffer;
	int readChars = 0;

	if (buffer != NULL) {
		if (sscanf(currentStr, "%*s %i %n", &readdata->ncols, &readChars) != 1) std::cout << "Reading DEM error!" << std::endl;
		currentStr += readChars;
		if (sscanf(currentStr, "%*s %i %n", &readdata->nrows, &readChars) != 1) std::cout << "Reading DEM error!" << std::endl;
		currentStr += readChars;
		if (sscanf(currentStr, "%*s %f %n", &readdata->xllcorner, &readChars) != 1) std::cout << "Reading DEM error!" << std::endl;
		currentStr += readChars;
		if (sscanf(currentStr, "%*s %f %n", &readdata->yllcorner, &readChars) != 1) std::cout << "Reading DEM error!" << std::endl;
		currentStr += readChars;
		if (sscanf(currentStr, "%*s %f %n", &readdata->cellsize, &readChars) != 1) std::cout << "Reading DEM error!" << std::endl;
		currentStr += readChars;
		if (sscanf(currentStr, "%*s %f %n", &readdata->NODATA_value, &readChars) != 1) std::cout << "Reading DEM error!" << std::endl;
		currentStr += readChars;

		readdata->max_value = readdata->NODATA_value;
		readdata->min_value = FLT_MAX;

		readdata->nelem = readdata->ncols * readdata->nrows;
		readdata->data.resize(getElem());

		float incoord = 0;

		for (int i = 0; i < getElem(); i++) {
			incoord = myStrtof(currentStr, &currentStr);

			if (incoord > readdata->max_value) {
				readdata->max_value = incoord;
			}
			if (incoord > readdata->NODATA_value + 1.0f && incoord < readdata->min_value) {
				readdata->min_value = incoord;
			}
			if (incoord == readdata->NODATA_value) {
				incoord = -1.0f;
			}

			readdata->data[i] = incoord;
		}
		terrainScale = readdata->max_value - readdata->min_value;

		free(buffer);
	} else {
		std::cerr << "Could not read file: " << inputfile << std::endl;
	}

	for (int i = 0; i < getElem(); i++) {
		if (readdata->data[i] > 0.0f) {
			readdata->data[i] -= readdata->min_value;
		}
	}

}

void DataHandler::normConvCompute() {

	normConvProgram = compileComputeShader("src/shaders/normConv.comp");
	glGenBuffers(3, normBuffers);

	GLint numData = getElem();
	GLuint reset = 0;
	//read buffer height
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, normBuffers[0]);	    //What data? getData()
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat)*numData, getData(), GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	//write buffer height
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, normBuffers[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat)*numData, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, normBuffers[2]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint), &reset, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	printError("init NormConvCompute");

	//END OF INIT

	glUseProgram(normConvProgram);
	glUniform2i(glGetUniformLocation(normConvProgram, "size"), getDataWidth(), getDataHeight());

	GLuint isNODATA = 1;
	while (isNODATA) {
		isNODATA = 0;

		glBindBuffersBase(GL_SHADER_STORAGE_BUFFER, 0, 3, normBuffers);
		glDispatchCompute((GLuint)ceil((GLfloat)getDataWidth() / 16.0f), (GLuint)ceil((GLfloat)getDataHeight() / 16.0f), 1);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, normBuffers[2]);
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &isNODATA);
		glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED, GL_UNSIGNED_INT, &reset);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		std::swap(normBuffers[0], normBuffers[1]);		
	}

	terrainBufferID = normBuffers[0];
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, terrainBufferID);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLfloat)*numData, getData());
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glDeleteBuffers(2, &normBuffers[1]);
}

GLfloat DataHandler::giveHeight(GLfloat x, GLfloat z) // Returns the height of a height map.
{
	int width = getDataWidth();
	int height = getDataHeight();

	int vertX1 = (int)floor(x);
	int vertZ1 = (int)floor(z);

	int vertX2 = (int)floor(x) + 1;
	int vertZ2 = (int)floor(z) + 1;

	int vertX3 = 0;
	int vertZ3 = 0;

	GLfloat yheight = 0;

	if ((vertX1 >= 0) && (vertZ1 >= 0) && (vertX2 < width) && (vertZ2 < height)) {

		GLfloat dist1 = vertX1 - x;
		GLfloat dist2 = vertZ1 - z;

		if (dist1 > dist2) {
			vertX3 = vertX1;
			vertZ3 = vertZ1 + 1;
		} else {
			vertX3 = vertX1 + 1;
			vertZ3 = vertZ1;
		}
		GLfloat vertY1 = getData()[(vertX1 + vertZ1 * width)];
		GLfloat vertY2 = getData()[(vertX2 + vertZ2 * width)];
		GLfloat vertY3 = getData()[(vertX3 + vertZ3 * width)];

		glm::vec3 p1 = { (float)vertX1 / (float)width, vertY1, (float)vertZ1 / (float)height };
		glm::vec3 p2 = { (float)vertX2 / (float)width, vertY2, (float)vertZ2 / (float)height };
		glm::vec3 p3 = { (float)vertX3 / (float)width, vertY3, (float)vertZ3 / (float)height };

		glm::vec3 planeNormal = { 0, 0, 0 };

		// This if/else might not be making any difference whatsoever.
		if (dist1 > dist2) {
			planeNormal = glm::normalize(glm::cross(p2 - p1, p3 - p1));
		} else {
			planeNormal = glm::normalize(glm::cross(p3 - p1, p2 - p1));
		}

		GLfloat D;
		D = glm::dot(planeNormal, p1);

		yheight = (D - planeNormal.x*x / width - planeNormal.z*z / height) / planeNormal.y;
	}
	return yheight;
}
















