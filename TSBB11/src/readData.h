/// @file readData.h
/// @brief Contains datatypes and functions to handle input terrain.

#ifndef READDATA_H
#define READDATA_H

#include "glm.hpp"
#include "loadobj.h"
#include "GL_utilities.h"

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
	float xllcorner; 		///< Dont know what this is.
	float yllcorner; 		///< Dont know what this is.
	float cellsize; 		///< Scaling for each cell, currently unused.
	float NODATA_value; ///< Value of input with no data, used to filter.
	float max_value; 		///< Maximum input, used to scale.
	float min_value;		///< Minimum input, used to scale.
	std::vector<float> data; ///< The terrain data is stored in a std::vector.
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
	int sampleFactor;

	// Variables for GPU processing
	GLuint plaintextureshader;	///< shader program to simple switch FBO
	GLuint filtershader;		///< shader program to perform LP filtering
	GLuint confidenceshader;	///< shader program to calculate confidence image
	GLuint combineshader;		///< shader program for combining into result
	FBOstruct *fbo1;			///< FBO for use during normalized convolution
	FBOstruct *fbo2;			///< FBO for use during normalized convolution
	FBOstruct *fbo3;			///< FBO for use during normalized convolution, data is loaded to this FBO
	Model* squareModel;			///< Canvas for GPU filtering

	// Data containers
	mapdata* readdata; 			///< mapdata struct for the loaded terrain data.
	Model* datamodel; 			///< model for the terrain data.

	// Just scaling
	GLfloat terrainScale;		///< Height scale for the terrain.

	/// @brief Reads the input data to the private mapdata struct.
	///
	/// Currently uses fscanf for better performance.
	/// @param inputfile path to DEM data file.
	/// @todo Possible performance improvement using fread instead.
	void readDEM(const char* inputfile);

	/// @brief Scales data while NODATA is present.
	///
	/// Input data will be scaled between 0.1 and 1.0 and NODATA will be given
	/// the value 0.0 so that input can be used in normalized convolution.
	///
	/// @see scaleDataAfter()
	/// @see performNormalizedConvolution()
	/// @see DataHandler()
	void scaleDataBefore();

	/// @brief Scales data when NODATA has been removed.
	///
	/// Used after normalized convolution to stretch data to 0.0 to 1.0.
	/// The data in the mapdata struct should already be scaled with
	/// scaleDataBefore() for this to work correctly.
	///
	/// @see scaleDataBefore()
	/// @see performNormalizedConvolution()
	void scaleDataAfter();

	/// @brief Makes a single pass of normalized convolution on the mapdata.
	///
	/// First filters the data with LP filter replacing only NODATA,
	/// second calculates confidence binary image, third filter the
	/// confidence image with LP to match first pass, fourth combine the
	/// confidence and initial pass to perform the normalized convolution.
	/// A final pass is performed to set the correct data in FBO's to simplify
	/// repeated use.
	/// @warning The data is assumed to be present in FBO3 before use as will
	/// the result after use, the data is NOT read from the GPU!
	void performGPUNormConv();

	
	/// @brief Generates terrain with the scale from terrainScale.
	///
	/// The output terrain is stored in the private model.
	/// Written by Ingemar Ragnemalm but slightly modfied
	/// @todo Currently no handling of the previous model is done if GenerateTerrain()
	/// is called multiple times.
	///
	/// @see giveNormal()
	/// @see giveHeight()
	void GenerateTerrain();

	/// @brief Gives a normal for a certain position in the terrain
	///
	/// Used by GenerateTerrain() to calculate normals for each vertex.
	/// Written by Ingemar Ragnemalm but slightly modfied
	/// @param x input width position
	/// @param y input height position
	/// @param z input depth position
	/// @param vertexArray input data
	/// @param indexArray input data
	/// @param width size of the input data
	/// @param height size of the input data
	/// @see GenerateTerrain()
	/// @see giveHeight()
	glm::vec3 giveNormal(int x, int y, int z, GLfloat *vertexArray, GLuint *indexArray, int width, int height);


public:
	

	/// @brief Reads DEM data, scales it and generates a model.
	///
	/// Reads the inputfile for DEM data and populates a mapdata struct,
	/// scales the data using scaleDataBefore() and creates the initial
	/// environment for Normalized convolution and finally generates an initial
	/// terrain.
	/// @param inputfile path to DEM data.
	/// @param sampleFactor Downsampling factor for the terrain. Must be a power of 2. [1,2,4...]
	/// default value is 1. (No downsampling).
	/// @param tScale sets terrainScale, default value is 500.0f
	/// @see readDEM()
	/// @see scaleDataBefore()
	/// @see GenerateTerrain()
	DataHandler(const char* inputfile, int sampleFactor = 1, GLfloat tScale = 500.0f);

	/// @brief Handle the internal pointers.
	~DataHandler();

	/// @brief Removes NODATA by normalized convolution.
	///
	/// Runs several passes of performGPUNormConv() until there are no more
	/// NODATA values present in the data. Runs 5 passes before reading the
	/// data back to the CPU and checking the data.
	/// When no NODATA is present scaleDataAfter() is performed and a new model is
	/// generated.
	/// @see performGPUNormConv()
	/// @see scaleDataAfter()
	/// @see GenerateTerrain()
	void performNormalizedConvolution();

	/// @brief Returns a datapoint from the mapdata.
	///
	/// Simplifies the getting of data by allowing access by coordinate since
	/// the data is stored in a linear vector.
	/// @param col input column
	/// @param row input row
	/// @return data(col,row)
	float getCoord(int col, int row);

	/// @brief Get a pointer to the mapdata data.
	///
	/// For uses like read all data to FBO or from GPU back to CPU.
	/// @return pointer to the beginning of the mapdata data.
	float* getData();

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
	GLfloat giveHeight(GLfloat x, GLfloat z, GLfloat *vertexArray, int width, int height);

	/// @brief Getter for the width of the data.
	/// @return the number of columns.
	int getWidth();

	/// @brief Getter for the height of the data.
	/// @return the number of rows.
	int getHeight();

	/// @brief Getter for the number of datapoints.
	/// @return columns * rows.
	int getElem();
	int getScale();

	/// @brief Pointer to the terrain model
	///
	/// Used to get the model for drawing in the main program
	/// @return Model to draw.
	Model *getModel();
};

#endif // READDATA_H
