#ifndef READDATA_H
#define READDATA_H

#include "glm.hpp"
#include "loadobj.h"

#include <vector>

struct mapdata {
	int	ncols;
	int nrows;
	int nelem;
	float xllcorner;
	float yllcorner;
	float cellsize;
	float NODATA_value;
	float max_value;
	float min_value;
	std::vector<float> data; 
};


class DataHandler 
{
private:
	mapdata* readdata;
	std::vector<Model*>* datamodel;
	GLfloat terrainScale;

	void readDEM(const char* inputfile);
	void scaleDataBefore();
	void scaleDataAfter();
	

	void performGPUNormConv();

	void GenerateTerrain();
	glm::vec3 giveNormal(glm::vec3 vertex, int x, int y, int z, GLfloat *vertexArray, GLuint *indexArray, int width, int height);
	GLfloat giveHeight(GLfloat x, GLfloat z, GLfloat *vertexArray, int width, int height);

public:
	DataHandler(const char* inputfile, GLfloat tScale = 500.0f);
	~DataHandler();

	void performNormalizedConvolution();

	float getCoord(int col, int row);
	float* getData();
	
	int getWidth();
	int getHeight();
	int getElem();

	std::vector<Model*>* getModel();
};

glm::vec3 giveNormal(glm::vec3 vertex,int x, int y, int z, GLfloat *vertexArray, GLuint *indexArray, int width, int height);
GLfloat giveHeight(GLfloat x, GLfloat z, GLfloat *vertexArray, int width, int height);

#endif // READDATA_H