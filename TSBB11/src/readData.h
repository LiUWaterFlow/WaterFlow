/// @file readData.h
/// @brief Contains datatypes and functions to handle input terrain.

#ifndef READDATA_H
#define READDATA_H

#include "loadobj.h"
#include "glm.hpp"
#include <vector>

/// @struct mapdata
/// @brief Contains information about the loaded terrain.
///
/// After loading the terrain the input will be stored in a mapdata struct.
/// The data should be rescaled after loading to lie between 0.0 and 1.0
/// to work correctly should you want to use any OpenGL filters on the data.
struct mapdata {
	int	ncols; 					///< Width of the terrain data.
	int nrows; 					///< Height of the terrain data.
	int nelem; 					///< Total number of terrain data.
	float xllcorner; 			///< Dont know what this is.
	float yllcorner; 			///< Dont know what this is.
	float cellsize; 			///< Scaling for each cell, currently unused.
	float NODATA_value;			///< Value of input with no data, used to filter.
	float max_value; 			///< Maximum input, used to scale.
	float min_value;			///< Minimum input, used to scale.
	std::vector<float> data;	///< The terrain data is stored in a std::vector.
};

/// @class DataHandler
/// @brief Loads and modifies terrain data for easier use.
///
/// Use the DataHandler class to import terrain data and generate terrain.
/// It automatically scales the data for easier filtering, and also generates
/// an initial terrain from the data after the data is loaded.
class DataHandler
{
private:
	// Data containers
	mapdata* readdata; 			///< mapdata struct for the loaded terrain data.
	GLuint textureUnit;			///< which texture unit to bind the height texture to.
	GLuint terrainTexture;		///< texture containing height terrain data.
	GLuint terrainBufferID;		///< terrainheight buffer corresponding to readdata data but on GPU.
	// Just scaling
	GLfloat terrainScale;		///< Height scale for the terrain. Calculated as the diff between min and max in the input data.

	/// @brief Reads the input data to the private mapdata struct.
	///
	/// Currently uses readFile to bulk read the file and then reads out the data info with sscanf and then another implementation
	/// of strtof to convert the data to floats.
	/// @param inputfile path to DEM data file.
	void readDEM(const char* inputfile);
	
	/// @brief Performs normalized convolution of the input data as a single computeshader.
	///
	/// At the end of the function terrainBufferID containins the buffer ID to the heightmap.
	/// The heightmap is also stored in the terrainTexture and will be bound to texture 3
	void normConvCompute();

public:
	/// @brief Reads DEM data, scales it and generates a model.
	///
	/// Reads the inputfile for DEM data and populates a mapdata struct,
	/// scales the data using scaleDataBefore() and creates the initial
	/// environment for Normalized convolution and finally generates an initial
	/// terrain.
	/// @param inputfile path to DEM data.
	/// @param sampleFactor Downsampling factor for the terrain. Must be a power of 2. [1,2,4...]
	/// default value is 1. (No downsampling). Note that the terrainScale will be used as
	/// tScale/sampleFactor. So a downsampling should not change the overall proportions of the model
	/// @param tScale sets terrainScale, default value is 500.0f
	/// @see readDEM()
	/// @see scaleDataBefore()
	/// @see GenerateTerrain()
	DataHandler(const char* inputfile, GLuint texUnit = 3);

	/// @brief Handle the internal pointers.
	~DataHandler();

	/// @brief Get a pointer to the mapdata data.
	///
	/// For uses like read all data to FBO or from GPU back to CPU.
	/// @return pointer to the beginning of the mapdata data.
	float* getData() { return &readdata->data[0]; }

	/// @brief Getter for the width of the data.
	/// @return the number of columns.
	int getDataWidth() { return readdata->ncols; }

	/// @brief Getter for the height of the data.
	/// @return the number of rows.
	int getDataHeight() { return readdata->nrows; }

	/// @brief Getter for the number of datapoints.
	/// @return columns * rows.
	int getElem() { return readdata->nelem; }

	/// @brief Get the terrain scale.
	/// @return Return a float of the diff between max and min sample in the data.
	GLfloat getTerrainScale() { return terrainScale; }

	/// @brief Get the texture unit for the terrain height.
	/// @return Returns the unit the terrain data texture is bound to.
	GLuint getTextureUnit() { return textureUnit; }

	/// @brief Get the texture id for the terrain height.
	/// @return Returns the ID for the terrain data texture.
	GLuint getTextureID() { return terrainTexture; }
	
	/// @brief Get the buffer ID containing the height data.
	/// @return Returns the gl buffer id to the buffer containing height data.
	GLuint getHeightBuffer() { return terrainBufferID; }

	/// @brief Returns a datapoint from the mapdata.
	///
	/// Simplifies the getting of data by allowing access by coordinate since
	/// the data is stored in a linear vector.
	/// @param col input column
	/// @param row input row
	/// @return data(col,row)
	float getCoord(int col, int row);

	/// @brief Gives a height for a certain position in the terrain
	///
	/// Used if the interpolated height data for a certain point is needed
	/// Written by Ingemar Ragnemalm but slightly modfied
	/// @param x input width position
	/// @param z input depth position
	/// @param vertexArray input data
	/// @param width size of the input data
	/// @param height size of the input data
	/// @see GenerateTerrain()
	/// @see giveNormal()
	GLfloat giveHeight(GLfloat x, GLfloat z);

	/// @brief Set a new texture unit to bind the height texture to
	void setTextureUnit(GLuint texUnit);
};

#endif // READDATA_H
