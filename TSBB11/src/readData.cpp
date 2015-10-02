#include "stdio.h"

#include "readData.h"

#include "GL_utilities.h"
#include "loadobj.h"
#include "glm.hpp"

#include <fstream>
#include <iostream>
#include <string>



using namespace std;

GLfloat square[] = { -1, -1, 0,
-1, 1, 0,
1, 1, 0,
1, -1, 0 };
GLfloat squareTexCoord[] = { 0, 0,
0, 1,
1, 1,
1, 0 };
GLuint squareIndices[] = { 0, 1, 2, 0, 2, 3 };

Model* squareModel;
FBOstruct *fbo1, *fbo2, *fbo3;
GLuint plaintextureshader = 0, filtershader = 0, confidenceshader = 0, combineshader = 0;
int tW, tH;

DataHandler::DataHandler(const char* inputfile, GLfloat tScale)
{
	readdata = new mapdata();
	terrainScale = tScale;

	datamodel = new std::vector<Model*>;


	cout << "Starting loading DEM data..." << endl;
	readDEM(inputfile);
	cout << "Finished loading DEM data..." << endl;

	cout << "Scaling DEM data..." << endl;
	scaleDataBefore();

	cout << "Compiling shaders..." << endl;

	plaintextureshader = loadShaders("src/shaders/plaintextureshader.vert", "src/shaders/plaintextureshader.frag");
	filtershader = loadShaders("src/shaders/plaintextureshader.vert", "src/shaders/filtershader.frag");
	confidenceshader = loadShaders("src/shaders/plaintextureshader.vert", "src/shaders/confidenceshader.frag");
	combineshader = loadShaders("src/shaders/plaintextureshader.vert", "src/shaders/combineshader.frag");

	tW = getWidth();
	tH = getHeight();

	fbo1 = initFBO3(tW, tH, NULL);
	fbo2 = initFBO3(tW, tH, NULL);
	fbo3 = initFBO3(tW, tH, getData());
	
	squareModel = LoadDataToModel(square, NULL, squareTexCoord, NULL, squareIndices, 4, 6);

	cout << "Generating terrain from DEM data..." << endl;
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

std::vector<Model*>* DataHandler::getModel()
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
	bool isNODATA = true;
	float min;
	while (isNODATA)
	{
		cout << "Starting five passes NC..." << endl;
		for (int i = 0; i < 5; i++)
			performGPUNormConv();

		glReadPixels(0, 0, tW, tH, GL_RED, GL_FLOAT, getData());

		cout << "Checking Data for NODATA..." << endl;
		isNODATA = false;
		
		min = 2.0f;
		for (int i = 0; i < getElem() && !isNODATA; i++)
		{
			float data = getData()[i];
			if (data < min)
				min = data;
			isNODATA = (data < 0.0001f);
		}
	}
	
	cout << "Scaling Data after NC..." << endl;
	scaleDataAfter();

	cout << "Generating new Model..." << endl;
	GenerateTerrain();
	cout << "Done generating new model..." << endl;

	// Reset initial GL inits
	useFBO(0L, fbo3, 0L);
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_TRUE);
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
	glUniform2f(glGetUniformLocation(filtershader, "in_size"), (float)tW, (float)tH);

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
	glUniform2f(glGetUniformLocation(filtershader, "in_size"), (float)tW, (float)tH);

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
	int twidth = getWidth();
	int theight = getHeight();
	int blockSize = 250;
	int widthBlocks = ceil(twidth / blockSize);
	int heightBlocks = ceil(theight / blockSize);
	for (int i = 0; i < widthBlocks; i++)
	{
		for (int j = 0; j < heightBlocks; j++)
		{
			int width = (twidth - i * blockSize > 0 ? blockSize +5 : twidth - (i - 1)*twidth);
			int height = (theight - i * blockSize > 0 ? blockSize +5 : theight - (i - 1)*theight);
			int blockSizeW = (twidth - i * blockSize > 0 ? blockSize : twidth - (i - 1)*twidth);
			int blockSizeH = (theight - i * blockSize > 0 ? blockSize : theight - (i - 1)*theight);

		
			int vertexCount = (width) * height;
			int triangleCount = (width - 1) * (height - 1) * 2;
			int x, z;

			GLfloat *vertexArray = (GLfloat *)malloc(sizeof(GLfloat) * 3 * vertexCount);
			GLfloat *normalArray = (GLfloat *)malloc(sizeof(GLfloat) * 3 * vertexCount);
			GLfloat *texCoordArray = (GLfloat *)malloc(sizeof(GLfloat) * 2 * vertexCount);
			GLuint *indexArray = (GLuint *)malloc(sizeof(GLuint)* triangleCount * 3);

			glm::vec3 tempNormal = { 0, 0, 0 };

			for (x = 0; x < width; x++)
			{
				for (z = 0; z < height; z++)
				{
					// Vertex array.
					vertexArray[(x + z * width) * 3 + 0] = x / 1.0f + blockSizeW*i;
					vertexArray[(x + z * width) * 3 + 1] = getCoord(x+blockSizeW*i, z+blockSizeH*j) * terrainScale; // Terrain height.
					vertexArray[(x + z * width) * 3 + 2] = z / 1.0f + blockSizeH*j;

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

			for (x = 0; x < width; x++)
			{
				for (z = 0; z < height; z++)
				{
					// Normal vectors.
					glm::vec3 vertex = { GLfloat(x + width*i), getCoord(x + width*i, z + width*j), GLfloat(z + width*j) };
					tempNormal = giveNormal(vertex,x,(int)getCoord(x + width*i, z + width*j), z , vertexArray, indexArray, width, height);
					normalArray[(x + z * width) * 3 + 0] = -tempNormal.x;
					normalArray[(x + z * width) * 3 + 1] = -tempNormal.y;
					normalArray[(x + z * width) * 3 + 2] = -tempNormal.z;
				}
			}

			// End of terrain generation.

			// Create Model and upload to GPU.
			datamodel->push_back(LoadDataToModel(
				vertexArray,
				normalArray,
				texCoordArray,
				NULL,
				indexArray,
				vertexCount,
				triangleCount * 3));
		}
	}
}

glm::vec3 DataHandler::giveNormal(glm::vec3 vertex,int x, int y, int z, GLfloat *vertexArray, GLuint *indexArray, int width, int height) // Returns the normal of a vertex.
{
	
	glm::vec3 normal = { 0, 1, 0 };

	glm::vec3 normal1 = { 0, 0, 0 };
	glm::vec3 normal2 = { 0, 0, 0 };
	glm::vec3 normal3 = { 0, 0, 0 };
	glm::vec3 normal4 = { 0, 0, 0 };
	glm::vec3 normal5 = { 0, 0, 0 };
	glm::vec3 normal6 = { 0, 0, 0 };

	if ((x > 1) && (z > 1) && (z < height - 2) && (x < width - 2))
	{
		glm::vec3 tempVec1={vertexArray[indexArray[((x - 1) + (z - 1) * (width - 1)) * 6 + 0] * 3],
							vertexArray[indexArray[((x - 1) + (z - 1) * (width - 1)) * 6 + 0] * 3 + 1],
							vertexArray[indexArray[((x - 1) + (z - 1) * (width - 1)) * 6 + 0] * 3 + 2] };

		glm::vec3 tempVec2={vertexArray[indexArray[((x)+(z - 1) * (width - 1)) * 6 + 0] * 3],
							vertexArray[indexArray[((x)+(z - 1) * (width - 1)) * 6 + 0] * 3 + 1],
							vertexArray[indexArray[((x)+(z - 1) * (width - 1)) * 6 + 0] * 3 + 2] };

		glm::vec3 tempVec3={vertexArray[indexArray[((x - 1) + (z)* (width - 1)) * 6 + 0] * 3],
							vertexArray[indexArray[((x - 1) + (z)* (width - 1)) * 6 + 0] * 3 + 1],
							vertexArray[indexArray[((x - 1) + (z)* (width - 1)) * 6 + 0] * 3 + 2] };

		glm::vec3 tempVec4={vertexArray[indexArray[((x + 1) + (z)* (width - 1)) * 6 + 0] * 3],
							vertexArray[indexArray[((x + 1) + (z)* (width - 1)) * 6 + 0] * 3 + 1],
							vertexArray[indexArray[((x + 1) + (z)* (width - 1)) * 6 + 0] * 3 + 2] };

		glm::vec3 tempVec5={vertexArray[indexArray[((x + 1) + (z + 1) * (width - 1)) * 6 + 0] * 3],
							vertexArray[indexArray[((x + 1) + (z + 1) * (width - 1)) * 6 + 0] * 3 + 1],
							vertexArray[indexArray[((x + 1) + (z + 1) * (width - 1)) * 6 + 0] * 3 + 2] };

		glm::vec3 tempVec6={vertexArray[indexArray[((x)+(z + 1) * (width - 1)) * 6 + 0] * 3],
							vertexArray[indexArray[((x)+(z + 1) * (width - 1)) * 6 + 0] * 3 + 1],
							vertexArray[indexArray[((x)+(z + 1) * (width - 1)) * 6 + 0] * 3 + 2] };


		normal1 = glm::cross(tempVec1 - vertex, tempVec2 - vertex);
		normal2 = glm::cross(tempVec3 - vertex, tempVec1 - vertex);
		glm::vec3 weighted1 = normalize(normalize(normal1) + normalize(normal2));
		normal3 = glm::cross(tempVec2 - vertex, tempVec4 - vertex);
		glm::vec3 weighted2 = normalize(normal3);
		normal4 = glm::cross(tempVec4 - vertex, tempVec5 - vertex);
		normal5 = glm::cross(tempVec5 - vertex, tempVec6 - vertex);
		glm::vec3 weighted3 = normalize(normalize(normal4) + normalize(normal5));
		normal6 = glm::cross(tempVec6 - vertex, tempVec3 - vertex);
		glm::vec3 weighted4 = normalize(normal6);

		normal = normalize(weighted1 + weighted2 + weighted3 + weighted4);

	}
	return normal;
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


