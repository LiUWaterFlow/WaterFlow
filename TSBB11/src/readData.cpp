#include "readData.h"

#include "VectorUtils3.h"
#include "loadobj.h"

#include <fstream>
#include <iostream>
#include <string>



using namespace std;

float DataHandler::getData(int col, int row)
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
	
		int numelem = readdata->ncols * readdata->nrows;
		for (int i = 0; i < numelem; i++)
		{
			infile >> incoord;
			readdata->data.push_back(incoord);
		}
		
    	infile.close();
	}
	else {
		cerr << "Could not open file: " << inputfile << endl;
	}
}

DataHandler::DataHandler(const char* inputfile, GLfloat tScale){
	readdata = new mapdata();
	readDEM(inputfile);
	GenerateTerrain(tScale);

	datamodel = GenerateTerrain(tScale);
}

DataHandler::~DataHandler()
{
	delete readdata;
}

Model* DataHandler::GenerateTerrain(GLfloat tScale) // Generates a model given a height map (grayscale .tga file for now).
{
	int width = readdata->ncols;
	int height = readdata->nrows;
	int vertexCount = width * height;
	int triangleCount = (width - 1) * (height - 1) * 2;
	int x, z;

	GLfloat *vertexArray = (GLfloat *)malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *normalArray = (GLfloat *)malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *texCoordArray = (GLfloat *)malloc(sizeof(GLfloat) * 2 * vertexCount);
	GLuint *indexArray = (GLuint *)malloc(sizeof(GLuint)* triangleCount * 3);

	vec3 tempNormal( 0, 0, 0 );

	for (x = 0; x < width; x++)
	{
		for (z = 0; z < height; z++)
		{
			// Vertex array.
			vertexArray[(x + z * width) * 3 + 0] = x / 1.0f;
			vertexArray[(x + z * width) * 3 + 1] = getData(x,z) / tScale; // Terrain height.
			vertexArray[(x + z * width) * 3 + 2] = z / 1.0f;

			// Texture coordinates.
			texCoordArray[(x + z * width) * 2 + 0] = (GLfloat)x;
			texCoordArray[(x + z * width) * 2 + 1] = (GLfloat)z;
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
			tempNormal = giveNormal(x, (int)(getData(x,z) / 10.0f), z, vertexArray, indexArray, width, height);
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

vec3 giveNormal(int x, int y, int z, GLfloat *vertexArray, GLuint *indexArray, int width, int height) // Returns the normal of a vertex.
{
	vec3 vertex( (GLfloat)x, (GLfloat)y, (GLfloat)z );
	vec3 normal( 0, 1, 0 );

	vec3 normal1( 0, 0, 0 );
	vec3 normal2( 0, 0, 0 );
	vec3 normal3( 0, 0, 0 );
	vec3 normal4( 0, 0, 0 );
	vec3 normal5( 0, 0, 0 );
	vec3 normal6( 0, 0, 0 );

	if ((x > 1) && (z > 1) && (z < height - 2) && (x < width - 2))
	{
		vec3 tempVec1( vertexArray[indexArray[((x - 1) + (z - 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x - 1) + (z - 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x - 1) + (z - 1) * (width - 1)) * 6 + 0] * 3 + 2] );

		vec3 tempVec2( vertexArray[indexArray[((x)+(z - 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x)+(z - 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x)+(z - 1) * (width - 1)) * 6 + 0] * 3 + 2] );

		vec3 tempVec3( vertexArray[indexArray[((x - 1) + (z)* (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x - 1) + (z)* (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x - 1) + (z)* (width - 1)) * 6 + 0] * 3 + 2] );

		vec3 tempVec4( vertexArray[indexArray[((x + 1) + (z)* (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x + 1) + (z)* (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x + 1) + (z)* (width - 1)) * 6 + 0] * 3 + 2] );

		vec3 tempVec5( vertexArray[indexArray[((x + 1) + (z + 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x + 1) + (z + 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x + 1) + (z + 1) * (width - 1)) * 6 + 0] * 3 + 2] );

		vec3 tempVec6( vertexArray[indexArray[((x)+(z + 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x)+(z + 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x)+(z + 1) * (width - 1)) * 6 + 0] * 3 + 2] );


		normal1 = CrossProduct(VectorSub(tempVec1, vertex), VectorSub(tempVec2, vertex));
		normal2 = CrossProduct(VectorSub(tempVec3, vertex), VectorSub(tempVec1, vertex));
		vec3 weighted1 = Normalize(VectorAdd(Normalize(normal1), Normalize(normal2)));
		normal3 = CrossProduct(VectorSub(tempVec2, vertex), VectorSub(tempVec4, vertex));
		vec3 weighted2 = Normalize(normal3);
		normal4 = CrossProduct(VectorSub(tempVec4, vertex), VectorSub(tempVec5, vertex));
		normal5 = CrossProduct(VectorSub(tempVec5, vertex), VectorSub(tempVec6, vertex));
		vec3 weighted3 = Normalize(VectorAdd(Normalize(normal4), Normalize(normal5)));
		normal6 = CrossProduct(VectorSub(tempVec6, vertex), VectorSub(tempVec3, vertex));
		vec3 weighted4 = Normalize(normal6);

		normal = Normalize(VectorAdd(weighted1, VectorAdd(weighted2, VectorAdd(weighted3, weighted4))));

	}
	return normal;
}

GLfloat giveHeight(GLfloat x, GLfloat z, GLfloat *vertexArray, int width, int height) // Returns the height of a height map.
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

		vec3 p1( vertexArray[(vertX1 + vertZ1 * width) * 3 + 0], vertY1, vertexArray[(vertX1 + vertZ1 * width) * 3 + 2] );
		vec3 p2( vertexArray[(vertX2 + vertZ2 * width) * 3 + 0], vertY2, vertexArray[(vertX2 + vertZ2 * width) * 3 + 2] );
		vec3 p3( vertexArray[(vertX3 + vertZ3 * width) * 3 + 0], vertY3, vertexArray[(vertX3 + vertZ3 * width) * 3 + 2] );

		vec3 planeNormal( 0, 0, 0 );

		// This if/else might not be making any difference whatsoever.
		if (dist1 > dist2)
		{
			planeNormal = Normalize(CrossProduct(VectorSub(p2, p1), VectorSub(p3, p1)));
		}
		else
		{
			planeNormal = Normalize(CrossProduct(VectorSub(p3, p1), VectorSub(p2, p1)));
		}

		GLfloat D;
		D = DotProduct(planeNormal, p1);

		yheight = (D - planeNormal.x*x - planeNormal.z*z) / planeNormal.y;
	}
	return yheight;
}

