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
	mapdata* readdata; 			///< mapdata struct for the loaded terrain data.
	GLuint terrainBufferID;		///< terrainheight buffer corresponding to readdata data but on GPU.
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
	/// Reads the inputfile for DEM data and populates a mapdata struct, and creates the buffer
	/// for Normalized convolution and finally generates an initial terrain height map.
	/// @param inputfile path to DEM data.
	/// @see readDEM()
	/// @see GenerateTerrain()
	DataHandler(const char* inputfile);

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
	/// @see GenerateTerrain()
	/// @see giveNormal()
	GLfloat giveHeight(GLfloat x, GLfloat z);
};

#endif // READDATA_H
