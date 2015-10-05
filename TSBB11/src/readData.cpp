#include "readData.h"

#include "loadobj.h"
#include "glm.hpp"

#include <fstream>
#include <iostream>
#include <string>



using namespace std;

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
		cout << "No mapdata exists!" << endl;
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

void DataHandler::readDEM(const char* inputfile)
{
	ifstream infile;
	infile.open(inputfile, ios::in);

	string intext;
	float incoord;

	if (infile.is_open())
	{
		infile >> intext >> readdata->ncols;
		infile >> intext >> readdata->nrows;
		infile >> intext >> readdata->xllcorner;
		infile >> intext >> readdata->yllcorner;
		infile >> intext >> readdata->cellsize;
		infile >> intext >> readdata->NODATA_value;

		readdata->max_value = readdata->NODATA_value;
		readdata->min_value = 20000000;

		readdata->nelem = readdata->ncols * readdata->nrows;

		for (int i = 0; i < readdata->nelem; i++)
		{
			infile >> incoord;

			if(incoord > readdata->max_value)
			{
				readdata->max_value = incoord;
			}
			if(incoord > readdata->NODATA_value + 1 && incoord < readdata->min_value)
			{
				readdata->min_value = incoord;
			}

			readdata->data.push_back(incoord);
		}

    	infile.close();
	}
	else {
		cerr << "Could not open file: " << inputfile << endl;
	}
}

void DataHandler::scaleData()
{
	for(auto i = readdata->data.begin(); i != readdata->data.end(); i++)
	{
		float diff = readdata->max_value - readdata->min_value;
		*i = ((*i - readdata->min_value) / diff) * 0.9f + 0.1f;
	}
}

DataHandler::DataHandler(const char* inputfile, GLfloat tScale){
	readdata = new mapdata();
	readDEM(inputfile);
	scaleData();
	datamodel = GenerateTerrain(tScale);
}


DataHandler::~DataHandler()
{
	delete readdata;
}

Model* DataHandler::GenerateTerrain(GLfloat tScale) // Generates a model given a height map (grayscale .tga file for now).
{
	int width = 100; // readdata->ncols;
	int height = 100; // readdata->nrows;
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
			vertexArray[(x + z * width) * 3 + 0] = x / 1.0;
			vertexArray[(x + z * width) * 3 + 1] = getCoord(x, z) * tScale; // Terrain height.
			vertexArray[(x + z * width) * 3 + 2] = z / 1.0;

			// Texture coordinates.
			texCoordArray[(x + z * width) * 2 + 0] = x;
			texCoordArray[(x + z * width) * 2 + 1] = z;
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
			tempNormal = giveNormal(x, (int)getCoord(x,z), z, vertexArray, indexArray, width, height);
			normalArray[(x + z * width) * 3 + 0] = -tempNormal.x;
			normalArray[(x + z * width) * 3 + 1] = -tempNormal.y;
			normalArray[(x + z * width) * 3 + 2] = -tempNormal.z;
		}
	}

	// End of terrain generation.

	// Create Model and upload to GPU.
	Model* model = LoadDataToModel(
		vertexArray,
		normalArray,
		texCoordArray,
		NULL,
		indexArray,
		vertexCount,
		triangleCount * 3);

	return model;
}

glm::vec3 giveNormal(int x, int y, int z, GLfloat *vertexArray, GLuint *indexArray, int width, int height) // Returns the normal of a vertex.
{
	glm::vec3 vertex = { GLfloat(x), GLfloat(y), GLfloat(z) };
	glm::vec3 normal = { 0, 1, 0 };

	glm::vec3 normal1 = { 0, 0, 0 };
	glm::vec3 normal2 = { 0, 0, 0 };
	glm::vec3 normal3 = { 0, 0, 0 };
	glm::vec3 normal4 = { 0, 0, 0 };
	glm::vec3 normal5 = { 0, 0, 0 };
	glm::vec3 normal6 = { 0, 0, 0 };

	if ((x > 1) && (z > 1) && (z < height - 2) && (x < width - 2))
	{
		glm::vec3 tempVec1 = { vertexArray[indexArray[((x - 1) + (z - 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x - 1) + (z - 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x - 1) + (z - 1) * (width - 1)) * 6 + 0] * 3 + 2] };

		glm::vec3 tempVec2 = { vertexArray[indexArray[((x)+(z - 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x)+(z - 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x)+(z - 1) * (width - 1)) * 6 + 0] * 3 + 2] };

		glm::vec3 tempVec3 = { vertexArray[indexArray[((x - 1) + (z)* (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x - 1) + (z)* (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x - 1) + (z)* (width - 1)) * 6 + 0] * 3 + 2] };

		glm::vec3 tempVec4 = { vertexArray[indexArray[((x + 1) + (z)* (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x + 1) + (z)* (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x + 1) + (z)* (width - 1)) * 6 + 0] * 3 + 2] };

		glm::vec3 tempVec5 = { vertexArray[indexArray[((x + 1) + (z + 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x + 1) + (z + 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x + 1) + (z + 1) * (width - 1)) * 6 + 0] * 3 + 2] };

		glm::vec3 tempVec6 = { vertexArray[indexArray[((x)+(z + 1) * (width - 1)) * 6 + 0] * 3],
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

GLfloat giveHeight(GLfloat x, GLfloat z, GLfloat *vertexArray, int width, int height) // Returns the height of a height map.
{
	GLfloat yheight = 0;

	int vertX1 = floor(x);
	int vertZ1 = floor(z);

	int vertX2 = floor(x) + 1;
	int vertZ2 = floor(z) + 1;

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
