/// @file readData.cpp
/// @brief Implementations of functions in readData.h

#include "stdio.h"

#include "readData.h"

/*
#include "GL_utilities.h"
#include "loadobj.h"
#include "glm.hpp"
*/
#include "common\GL_utilities.h"
#include "common\loadobj.h"
#include "common\glm\glm.hpp"
#include "common\Windows\wglew.h"
#include "common\Windows\glew.h"

#include <fstream>
#include <iostream>
#include <string>


using namespace std;


DataHandler::DataHandler(const char* inputfile,int sampleFactor)
{
	datamodel = NULL;
	readdata = new mapdata();
	this->sampleFactor = sampleFactor;

	readDEM(inputfile);
	scaleDataBefore();


	// Create canvas to draw on
	GLfloat square[] = { -1, -1, 0,
		-1, 1, 0,
		1, 1, 0,
		1, -1, 0 };
	GLfloat squareTexCoord[] = { 0, 0,
		0, 1,
		1, 1,
		1, 0 };
	GLuint squareIndices[] = { 0, 1, 2, 0, 2, 3 };
	squareModel = LoadDataToModel(square, NULL, squareTexCoord, NULL, squareIndices, 4, 6);

	performNormalizedConvolution();

	GenerateTerrain();
}

DataHandler::~DataHandler()
{
	delete readdata;
}

float DataHandler::getCoord(int col, int row)
{
	int index;
	float retdata = 0;

	if (readdata != NULL) {

		if(col < readdata->ncols && row < readdata->nrows)
		{
			index = col + row * readdata->ncols;
		}
		else
		{
			cerr << "Input does not exist in data." << endl;
			index = 0;
		}
		retdata = readdata->data[index];
	}
	else {
		cerr << "No mapdata exists." << endl;
	}

	return retdata;
}

int DataHandler::getSampleFactor()
{
	return sampleFactor;
}

float* DataHandler::getData()
{
	return &readdata->data[0];
}

int DataHandler::getWidth()
{
	return readdata->ncols;
}

int DataHandler::getHeight()
{
	return readdata->nrows;
}

int DataHandler::getElem()
{
	return readdata->nelem;
}

GLfloat DataHandler::getTerrainScale()
{
	return terrainScale;
}

Model* DataHandler::getModel()
{
	return datamodel;
}

void DataHandler::readDEM(const char* inputfile)
{
	FILE* file = fopen(inputfile, "r");

	char intext [80];
	float incoord = 0;

	if (file != NULL)
	{
		fscanf(file, "%s %i", &intext, &readdata->ncols);
		fscanf(file, "%s %i", &intext, &readdata->nrows);
		fscanf(file, "%s %f", &intext, &readdata->xllcorner);
		fscanf(file, "%s %f", &intext, &readdata->yllcorner);
		fscanf(file, "%s %f", &intext, &readdata->cellsize);
		fscanf(file, "%s %f", &intext, &readdata->NODATA_value);

		readdata->max_value = readdata->NODATA_value;
		readdata->min_value = 20000000;

		readdata->nelem = readdata->ncols * readdata->nrows;
		readdata->data.resize(getElem());

		int nRead = 0;
		for (int i = 0; i < getElem(); i++)
		{
			nRead = fscanf(file, "%f", &incoord);

			if (nRead != 1)
			{
				cerr << "Less values than it should be!" << endl;
				break;
			}

			if (incoord > readdata->max_value)
			{
				readdata->max_value = incoord;
			}
			if (incoord > readdata->NODATA_value + 1.0f && incoord < readdata->min_value)
			{
				readdata->min_value = incoord;
			}

			readdata->data[i] = incoord;
		}

		terrainScale = readdata->max_value - readdata->min_value;

		fclose(file);
	}
	else {
		cerr << "Could not open file: " << inputfile << endl;
	}
}

// Will scale the data so that data before normalized convolution is between 0.0 and 1.0
void DataHandler::scaleDataBefore()
{
	for (int i = 0; i < getElem(); i++)
	{
		float diff = readdata->max_value - readdata->min_value;

		// Scale real data between 0.1 and 1.0
		getData()[i] = ((getData()[i] - readdata->min_value) / diff) * 0.9f + 0.1f;

		// Set nodata to 0
		if (getData()[i] < 0.05)
			getData()[i] = 0.0f;
	}
}

// Data after normalized convolution should be between 0.1 and 1.0
void DataHandler::scaleDataAfter()
{
	for (int i = 0; i < getElem(); i++)
	{
		// Scale real data between 0.0 and 1.0
		getData()[i] = (getData()[i] - 0.1f) / 0.9f;
	}
}


void DataHandler::performNormalizedConvolution()
{
	plaintextureshader = loadShaders("src/shaders/plaintextureshader.vert", "src/shaders/plaintextureshader.frag");
	filtershader = loadShaders("src/shaders/plaintextureshader.vert", "src/shaders/filtershader.frag");
	confidenceshader = loadShaders("src/shaders/plaintextureshader.vert", "src/shaders/confidenceshader.frag");
	combineshader = loadShaders("src/shaders/plaintextureshader.vert", "src/shaders/combineshader.frag");

	//extract screen size
	GLint viewport[4] = {0,0,0,0};
	GLint w, h;
	glGetIntegerv(GL_VIEWPORT, viewport);
	w = viewport[2] - viewport[0];
	h = viewport[3] - viewport[1];



	// Initialize the FBO's
	fbo1 = initFBO3(getWidth(), getHeight(), NULL);
	fbo2 = initFBO3(getWidth(), getHeight(), NULL);
	fbo3 = initFBO3(getWidth(), getHeight(), getData());

	// Perform normalized convolution until no more NODATA
	bool isNODATA = true;
	while (isNODATA)
	{
		for (int i = 0; i < 5; i++)
			performGPUNormConv();

		glReadPixels(0, 0, getWidth(), getHeight(), GL_RED, GL_FLOAT, getData());

		isNODATA = false;
		for (int i = 0; i < getElem() && !isNODATA; i++)
		{
			isNODATA = (getData()[i] < 0.0001f);
		}
	}

	scaleDataAfter();

	releaseFBO(fbo1);
	delete fbo1;
	releaseFBO(fbo2);
	delete fbo2;
	releaseFBO(fbo3);
	delete fbo3;
	glDeleteProgram(plaintextureshader);
	glDeleteProgram(filtershader);
	glDeleteProgram(confidenceshader);
	glDeleteProgram(combineshader);

	// Reset to initial GL inits
	useFBO(0L, 0L, 0L);
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_TRUE);
	// Reset render area
	glViewport(0, 0, w, h);


}

void DataHandler::performGPUNormConv()
{
	// Filter original
	useFBO(fbo1, fbo3, 0L);

	// Clear framebuffer & zbuffer
	glClearColor(0.1f, 0.1f, 0.3f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate shader program
	glUseProgram(filtershader);
	glUniform2f(glGetUniformLocation(filtershader, "in_size"), (float)getWidth(), (float)getHeight());

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	DrawModel(squareModel, filtershader, "in_Position", NULL, "in_TexCoord");

	// Create confidence
	useFBO(fbo2, fbo3, 0L);

	// Clear framebuffer & zbuffer
	glClearColor(0.1f, 0.1f, 0.3f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate shader program
	glUseProgram(confidenceshader);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	DrawModel(squareModel, confidenceshader, "in_Position", NULL, "in_TexCoord");

	// Filter confidence
	useFBO(fbo3, fbo2, 0L);

	// Clear framebuffer & zbuffer
	glClearColor(0.1f, 0.1f, 0.3f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate shader program
	glUseProgram(filtershader);
	glUniform2f(glGetUniformLocation(filtershader, "in_size"), (float)getWidth(), (float)getHeight());

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	DrawModel(squareModel, filtershader, "in_Position", NULL, "in_TexCoord");

	// Combine
	useFBO(fbo2, fbo1, fbo3);

	// Clear framebuffer & zbuffer
	glClearColor(0.1f, 0.1f, 0.3f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate shader program
	glUseProgram(combineshader);
	glUniform1i(glGetUniformLocation(combineshader, "dataTex"), 0);
	glUniform1i(glGetUniformLocation(combineshader, "confTex"), 1);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	DrawModel(squareModel, combineshader, "in_Position", NULL, "in_TexCoord");

	// Swap FBOs
	useFBO(fbo3, fbo2, 0L);

	// Clear framebuffer & zbuffer
	glClearColor(0.1f, 0.1f, 0.3f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate shader program
	glUseProgram(plaintextureshader);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	DrawModel(squareModel, plaintextureshader, "in_Position", NULL, "in_TexCoord");
}

void DataHandler::GenerateTerrain()
{
	//remove all data from previous model before generating a new one.
	if (datamodel != NULL)
	{
		delete datamodel->vertexArray;
		delete datamodel->normalArray;
		delete datamodel->texCoordArray;
		delete datamodel->colorArray; // Rarely used
		delete datamodel->indexArray;
		delete datamodel;
	}

	//Reduce width and height with samplefactor
	int width = (int)floor(getWidth()/sampleFactor);
	int height = (int)floor(getHeight()/sampleFactor);

	int vertexCount = width * height;
	int triangleCount = (width - 1) * (height - 1) * 2;
	int x, z;

	GLfloat *vertexArray = (GLfloat *)malloc(sizeof(GLfloat)* 3 * vertexCount);
	GLfloat *normalArray = (GLfloat *)malloc(sizeof(GLfloat)* 3 * vertexCount);
	GLfloat *texCoordArray = (GLfloat *)malloc(sizeof(GLfloat)* 2 * vertexCount);
	GLuint *indexArray = (GLuint *)malloc(sizeof(GLuint)* triangleCount * 3);

	glm::vec3 tempNormal = { 0, 0, 0 };

	for (x = 0; x < width; x++)
	{
		for (z = 0; z < height; z++)
		{
			// Vertex array.
			vertexArray[(x + z * width) * 3 + 0] = (float)x / (float)width;
			vertexArray[(x + z * width) * 3 + 1] = getCoord(x*sampleFactor, z*sampleFactor);
			vertexArray[(x + z * width) * 3 + 2] = (float)z / (float)height;

			// Texture coordinates.
			texCoordArray[(x + z * width) * 2 + 0] = (float)x;
			texCoordArray[(x + z * width) * 2 + 1] = (float)z;
		}
	}

	for (x = 0; x < width - 1; x++)
	{
		for (z = 0; z < height - 1; z++)
		{
			// Triangle 1.
			indexArray[(x + z * (width - 1)) * 6 + 0] = x + z * width;
			indexArray[(x + z * (width - 1)) * 6 + 1] = x + (z + 1) * width;
			indexArray[(x + z * (width - 1)) * 6 + 2] = x + 1 + z * width;
			// Triangle 2.
			indexArray[(x + z * (width - 1)) * 6 + 3] = x + 1 + z * width;
			indexArray[(x + z * (width - 1)) * 6 + 4] = x + (z + 1) * width;
			indexArray[(x + z * (width - 1)) * 6 + 5] = x + 1 + (z + 1) * width;
		}
	}

	calculateNormalsGPU(vertexArray, normalArray, width, height);

	// End of terrain generation.

	// Create Model and upload to GPU.
	datamodel = LoadDataToModel(
		vertexArray,
		normalArray,
		texCoordArray,
		NULL,
		indexArray,
		vertexCount,
		triangleCount * 3);
}

GLfloat DataHandler::giveHeight(GLfloat x, GLfloat z, GLfloat *vertexArray, int width, int height) // Returns the height of a height map.
{
	GLfloat yheight = 0;

	int vertX1 = (int)floor(x);
	int vertZ1 = (int)floor(z);

	int vertX2 = (int)floor(x) + 1;
	int vertZ2 = (int)floor(z) + 1;

	int vertX3 = 0;
	int vertZ3 = 0;

	if ((vertX1 > 1) && (vertZ1 > 1) && (vertX2 < height - 2) && (vertZ2 < width - 2))
	{

		GLfloat dist1 = vertX1 - x;
		GLfloat dist2 = vertZ1 - z;

		if (dist1 > dist2)
		{
			vertX3 = vertX1;
			vertZ3 = vertZ1 + 1;
		}
		else
		{
			vertX3 = vertX1 + 1;
			vertZ3 = vertZ1;
		}
		GLfloat vertY1 = vertexArray[(vertX1 + vertZ1 * width) * 3 + 1];

		GLfloat vertY2 = vertexArray[(vertX2 + vertZ2 * width) * 3 + 1];

		GLfloat vertY3 = vertexArray[(vertX3 + vertZ3 * width) * 3 + 1];

		glm::vec3 p1 = { vertexArray[(vertX1 + vertZ1 * width) * 3 + 0], vertY1, vertexArray[(vertX1 + vertZ1 * width) * 3 + 2] };
		glm::vec3 p2 = { vertexArray[(vertX2 + vertZ2 * width) * 3 + 0], vertY2, vertexArray[(vertX2 + vertZ2 * width) * 3 + 2] };
		glm::vec3 p3 = { vertexArray[(vertX3 + vertZ3 * width) * 3 + 0], vertY3, vertexArray[(vertX3 + vertZ3 * width) * 3 + 2] };

		glm::vec3 planeNormal = { 0, 0, 0 };

		// This if/else might not be making any difference whatsoever.
		if (dist1 > dist2)
		{
			planeNormal = normalize(glm::cross(p2 - p1, p3 - p1));
		}
		else
		{
			planeNormal = normalize(glm::cross(p3 - p1, p2 - p1));
		}

		GLfloat D;
		D = glm::dot(planeNormal, p1);

		yheight = (D - planeNormal.x*x - planeNormal.z*z) / planeNormal.y;
	}
	return yheight;
}

void DataHandler::calculateNormalsGPU(GLfloat *vertexArray, GLfloat *normalArray, int width, int height)
{
	normalshader = loadShaders("src/shaders/plaintextureshader.vert", "src/shaders/normalshader.frag");

	//extract screen size
	GLint viewport[4] = {0,0,0,0};
	GLint w, h;
	glGetIntegerv(GL_VIEWPORT, viewport);
	w = viewport[2] - viewport[0];
	h = viewport[3] - viewport[1];

	// Initialize the FBO's
	fbo4 = initFBO4(width, height, vertexArray);
	fbo5 = initFBO4(width, height, NULL);

	// Filter original
	useFBO(fbo5, fbo4, 0L);

	// Clear framebuffer & zbuffer
	glClearColor(0.1f, 0.1f, 0.3f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate shader program
	glUseProgram(normalshader);
	glUniform2f(glGetUniformLocation(normalshader, "in_size"), (float)width, (float)height);
	glUniform1f(glGetUniformLocation(normalshader, "in_sample"), (float)sampleFactor);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	DrawModel(squareModel, normalshader, "in_Position", NULL, "in_TexCoord");

	glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT, normalArray);

	releaseFBO(fbo4);
	delete fbo4;
	releaseFBO(fbo5);
	delete fbo5;
	glDeleteProgram(normalshader);

	// Reset to initial GL inits

	useFBO(0L, 0L, 0L);
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_TRUE);

	// Reset render area
	glViewport(0, 0, w, h);


}
