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
	mapdata* readdata;
	
	void readDEM(const char* inputfile);
	void scaleData();
	Model* GenerateTerrain(GLfloat tScale);

public:
	Model* datamodel;

	DataHandler(const char* inputfile, GLfloat tScale = 500.0f);
	~DataHandler();
	float getCoord(int col, int row);
	float* getData();
	
	int getWidth();
	int getHeight();
	int getElem();
	
};

glm::vec3 giveNormal(int x, int y, int z, GLfloat *vertexArray, GLuint *indexArray, int width, int height);
GLfloat giveHeight(GLfloat x, GLfloat z, GLfloat *vertexArray, int width, int height);
