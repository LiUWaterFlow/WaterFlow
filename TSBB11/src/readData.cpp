/// @file readData.cpp
/// @brief Implementations of functions in readData.h

#include "stdio.h"

#include "readData.h"

#include "GL_utilities.h"
#include "loadobj.h"
#include "glm.hpp"
#include "Utilities.h"

#include <fstream>
#include <iostream>
#include <string>


using namespace std;

// ===== Constructors and destructors

DataHandler::DataHandler(const char* inputfile,int sampleFactor, int blockSize)
: sampleFactor(sampleFactor), blockSize(blockSize)
{
	readdata = new mapdata();
	datamodel = new std::vector<Model*>;

	readDEM(inputfile);
	scaleDataBefore();
	performNormalizedConvolution();
	scaleDataAfter();
	GenerateTerrain();
}

DataHandler::~DataHandler()
{
	delete readdata;

	while (datamodel->size())
	{
		Model* tmp = datamodel->back();
		datamodel->pop_back();
		releaseModel(tmp);
	}
}

// ===== Getters =====

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
			cerr << "Input does not exist in data. Col: " << col <<" Row: " << row << endl;
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
int DataHandler::getDataWidth()
{
	return readdata->ncols;
}
int DataHandler::getModelWidth()
{
	return (int)floor(readdata->ncols / sampleFactor);
}
int DataHandler::getDataHeight()
{
	return readdata->nrows;
}
int DataHandler::getModelHeight()
{
	return (int)floor(readdata->nrows / sampleFactor);
}
int DataHandler::getElem()
{
	return readdata->nelem;
}
GLfloat DataHandler::getTerrainScale()
{
	return terrainScale;
}
std::vector<Model*>* DataHandler::getModel()
{
	return datamodel;
}

// ===== Actual functions =====

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

void DataHandler::performNormalizedConvolution()
{
	// Create the filters
	GLuint filtershader = loadShaders("src/shaders/plaintextureshader.vert", "src/shaders/filtershader.frag");
	GLuint confidenceshader = loadShaders("src/shaders/plaintextureshader.vert", "src/shaders/confidenceshader.frag");
	GLuint combineshader = loadShaders("src/shaders/plaintextureshader.vert", "src/shaders/combineshader.frag");

	// Create somewhere to draw the data.
	Model* squareModel = generateCanvas();

	// Initialize the FBO's
	FBOstruct *fbo1 = initFBO3(getDataWidth(), getDataHeight(), NULL);
	FBOstruct *fbo2 = initFBO3(getDataWidth(), getDataHeight(), NULL);
	FBOstruct *fbo3 = initFBO3(getDataWidth(), getDataHeight(), getData());

	// Perform normalized convolution until no more NODATA
	bool isNODATA = true;
	while (isNODATA)
	{
		for (int i = 0; i < 5; i++)
		{
			// Filter original
			useFBO(fbo1, fbo3, 0L);

			// Clear framebuffer & zbuffer
			glClearColor(0.1f, 0.1f, 0.3f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Activate shader program
			glUseProgram(filtershader);
			glUniform2f(glGetUniformLocation(filtershader, "in_size"), (float)getDataWidth(), (float)getDataHeight());

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
			glUniform2f(glGetUniformLocation(filtershader, "in_size"), (float)getDataWidth(), (float)getDataHeight());

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
			FBOstruct* temp = fbo2;
			fbo2 = fbo3;
			fbo3 = temp;
		}

		glReadPixels(0, 0, getDataWidth(), getDataHeight(), GL_RED, GL_FLOAT, getData());

		isNODATA = false;
		for (int i = 0; i < getElem() && !isNODATA; i++)
		{
			isNODATA = (getData()[i] < 0.0001f);
		}
	}

	// Reset to initial GL inits
	useFBO(0L, 0L, 0L);
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_TRUE);

	// Cleanup
	glDeleteProgram(filtershader);
	glDeleteProgram(confidenceshader);
	glDeleteProgram(combineshader);
	releaseFBO(fbo1); // Must be after useFBO reset or canvas size will get all weird
	releaseFBO(fbo2);
	releaseFBO(fbo3);
	releaseModel(squareModel);
}

void DataHandler::scaleDataAfter()
{
	for (int i = 0; i < getElem(); i++)
	{
		// Scale real data between 0.0 and 1.0
		getData()[i] = (getData()[i] - 0.1f) / 0.9f;
	}
}

void DataHandler::GenerateTerrain()
{
	//Create the whole model for normal calculation.
	GLuint preCalcWidth, preCalcHeight, preCalcVertexC;
	preCalcWidth = getModelWidth();
	preCalcHeight = getModelHeight();
	preCalcVertexC = preCalcWidth*preCalcHeight;

	GLfloat *preCalcVertexArray = (GLfloat *)malloc(sizeof(GLfloat) * 3 * preCalcVertexC);
	GLfloat *preCalcNormalArray = (GLfloat *)malloc(sizeof(GLfloat) * 3 * preCalcVertexC);

	for (GLuint x = 0; x < preCalcWidth; x++)
	{
		for (GLuint z = 0; z < preCalcHeight; z++)
		{

			preCalcVertexArray[(x + z * preCalcWidth) * 3 + 0] = (float)(x) / (float)preCalcWidth;
			preCalcVertexArray[(x + z * preCalcWidth) * 3 + 1] = getCoord((x)*sampleFactor, (z)*sampleFactor);
			preCalcVertexArray[(x + z * preCalcWidth) * 3 + 2] = (float)(z) / (float)preCalcHeight;
		}
	}

	calculateNormalsGPU(preCalcVertexArray, preCalcNormalArray, preCalcWidth, preCalcHeight);

	int twidth = preCalcWidth;
	int theight = preCalcHeight;
	int widthBlocks = (int)ceil(GLfloat(twidth) / GLfloat(blockSize));
	int heightBlocks = (int)ceil(GLfloat(theight) / GLfloat(blockSize));
	for (int i = 0; i < widthBlocks; i++)
	{
		for (int j = 0; j < heightBlocks; j++)
		{
			int width =(twidth -(i+1)*blockSize > 0 ? blockSize+2 : twidth - i*blockSize);
			int height =(theight -(j+1)*blockSize > 0 ? blockSize+2 : theight - j*blockSize);

			int vertexCount = width * height;
			int triangleCount = (width - 1) * (height - 1) * 2;
			int x, z;

			GLfloat *vertexArray = (GLfloat *)malloc(sizeof(GLfloat) * 3 * vertexCount);
			GLfloat *normalArray = (GLfloat *)malloc(sizeof(GLfloat) * 3 * vertexCount);
			GLfloat *texCoordArray = (GLfloat *)malloc(sizeof(GLfloat) * 2 * vertexCount);
			GLuint *indexArray = (GLuint *)malloc(sizeof(GLuint)* triangleCount * 3);

			for (x = 0; x < width; x++)
			{
				for (z = 0; z < height; z++)
				{
					// Vertex array.
					vertexArray[(x + z * width) * 3 + 0] = preCalcVertexArray[((x + blockSize*i) + (z + blockSize*j)* preCalcWidth) * 3 + 0];
					vertexArray[(x + z * width) * 3 + 1] = preCalcVertexArray[((x + blockSize*i) + (z + blockSize*j)* preCalcWidth) * 3 + 1];
					vertexArray[(x + z * width) * 3 + 2] = preCalcVertexArray[((x + blockSize*i) + (z + blockSize*j)* preCalcWidth) * 3 + 2];

					// Texture coordinates.
					texCoordArray[(x + z * width) * 2 + 0] = (float)x;
					texCoordArray[(x + z * width) * 2 + 1] = (float)z;

					//Insert normals from the precalculated normals.
					normalArray[(x + z * width) * 3 + 0] = preCalcNormalArray[((x + blockSize*i) + (z + blockSize*j)* preCalcWidth) * 3 + 0];
					normalArray[(x + z * width) * 3 + 1] = preCalcNormalArray[((x + blockSize*i) + (z + blockSize*j)* preCalcWidth) * 3 + 1];
					normalArray[(x + z * width) * 3 + 2] = preCalcNormalArray[((x + blockSize*i) + (z + blockSize*j) *preCalcWidth) * 3 + 2];

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

			// Create Model and upload to GPU.
			datamodel->push_back(LoadDataToModel(	vertexArray,
													normalArray,
													texCoordArray,
													NULL,
													indexArray,
													vertexCount,
													triangleCount * 3));

			//Should be safe to delete since own space is created in the model.
			free(indexArray);
			free(normalArray);
			free(texCoordArray);
			free(vertexArray);
		}
	}
	//Delete the preCalculated normals and vertices
	free(preCalcVertexArray);
	free(preCalcNormalArray);
}

void DataHandler::calculateNormalsGPU(GLfloat *vertexArray, GLfloat *normalArray, int width, int height)
{
	GLuint normalshader = loadShaders("src/shaders/plaintextureshader.vert", "src/shaders/normalshader.frag");

	Model* squareModel = generateCanvas();

	// Initialize the FBO's
	FBOstruct *fbo4 = initFBO4(width, height, vertexArray);
	FBOstruct *fbo5 = initFBO4(width, height, NULL);

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

	// Reset to initial GL inits
	useFBO(0L, 0L, 0L);
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_TRUE);

	// Cleanup
	glDeleteProgram(normalshader);
	releaseFBO(fbo4); // Must be after useFBO reset or canvas size will get all weird
	releaseFBO(fbo5);
	releaseModel(squareModel);
}

GLfloat DataHandler::giveHeight(GLfloat x, GLfloat z) // Returns the height of a height map.
{
	int width = getModelWidth();
	int height = getModelHeight();

	int vertX1 = (int)floor(x);
	int vertZ1 = (int)floor(z);

	int vertX2 = (int)floor(x) + 1;
	int vertZ2 = (int)floor(z) + 1;

	int vertX3 = 0;
	int vertZ3 = 0;

	GLfloat yheight = 0;

	if ((vertX1 >= 0) && (vertZ1 >= 0) && (vertX2 <= width-1) && (vertZ2 <= height-1))
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
		GLfloat vertY1 = getData()[(vertX1 + vertZ1 * width)];
		GLfloat vertY2 = getData()[(vertX2 + vertZ2 * width)];
		GLfloat vertY3 = getData()[(vertX3 + vertZ3 * width)];

		glm::vec3 p1 = { (float)vertX1 / (float)width, vertY1, (float)vertZ1 / (float)height };
		glm::vec3 p2 = { (float)vertX2 / (float)width, vertY2, (float)vertZ2 / (float)height };
		glm::vec3 p3 = { (float)vertX3 / (float)width, vertY3, (float)vertZ3 / (float)height };

		glm::vec3 planeNormal = { 0, 0, 0 };

		// This if/else might not be making any difference whatsoever.
		if (dist1 > dist2)
		{
			planeNormal = glm::normalize(glm::cross(p2 - p1, p3 - p1));
		}
		else
		{
			planeNormal = glm::normalize(glm::cross(p3 - p1, p2 - p1));
		}

		GLfloat D;
		D = glm::dot(planeNormal, p1);

		yheight = (D - planeNormal.x*x/width - planeNormal.z*z/height) / planeNormal.y;
		yheight *= getTerrainScale();
	}
	return yheight;
}
