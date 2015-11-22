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
	std::vector<Model*>* datamodel; 			///< model for the terrain data.
	GLuint terrainTexture;		///< texture containing normals and height terrain data.
	GLuint terrainBufferID;		///< terrainheight buffer corresponding to readdata data but on GPU.
	// Just scaling
	GLfloat terrainScale;		///< Height scale for the terrain. Calculated as the diff between min and max in the input data.
	int sampleFactor;			///< Sample factor used for constructing model.
	int blockSize;				///< Size of each model block.

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

	/// @brief Removes NODATA by normalized convolution.
	///
	/// Runs several passes of performGPUNormConv() until there are no more
	/// NODATA values present in the data. Runs 5 passes before reading the
	/// data back to the CPU and checking the data.
	/// When no NODATA is present scaleDataAfter() is performed and a new model is
	/// generated. First filters the data with LP filter replacing only NODATA,
	/// second calculates confidence binary image, third filter the
	/// confidence image with LP to match first pass, fourth combine the
	/// confidence and initial pass to perform the normalized convolution.
	/// A final pass is performed to set the correct data in FBO's to simplify
	/// repeated use.
	/// @see performGPUNormConv()
	/// @see scaleDataAfter()
	/// @see GenerateTerrain()
	void performNormalizedConvolution();

	/// @brief Scales data when NODATA has been removed.
	///
	/// Used after normalized convolution to stretch data to 0.0 to 1.0.
	/// The data in the mapdata struct should already be scaled with
	/// scaleDataBefore() for this to work correctly.
	///
	/// @see scaleDataBefore()
	/// @see performNormalizedConvolution()
	void scaleDataAfter();

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

	/// @brief Calculates the normals for the whole terrain on the GPU.
	///
	/// The normals are calculated using a 3x3 sobel filter in x and z direction and then normalized. This is used in GenerateTerrain() to speed up the normal calculations.
	/// @param normalArray this is the normalArray for the model that should be used.
	/// @see GenerateTerrain()
	/// @todo The scaling for the y component is currently arbitrary and might need some investigation if it should scale with some parameter.
	void calculateNormalsGPU(GLfloat *vertexArray, GLfloat *normalArray, int width, int height);

	

public:
	GLuint normConvProgram;
	GLuint normBuffers[3];

	GLuint normalsProgram;
	GLuint normalsBuffer;

	///@brief should not be public.
	///@todo Make private!	
	GLuint computeProgram;
	GLuint computeBuffers[4];  /// 0 is pos, 1 is texcoords, 2 is indices,3 normals.

	GLuint numIndices;
	void calculateNormalsCompute();

	/// @brief Performes the same operation as performNormalizedConvolution but as a single computeshader.
	///
	/// At the end of the function terrainHeight will be a buffer ID containing the heightmap.
	/// @see performNormalizedConvolution()
	void normConvCompute();



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
	DataHandler(const char* inputfile, int sampleFactor = 1, int blockSize = 250);

	/// @brief Handle the internal pointers.
	~DataHandler();

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
	GLfloat giveHeight(GLfloat x, GLfloat z);

	/// @brief Getter for the width of the data.
	/// @return the number of columns.
	int getDataWidth();

	/// @brief Getter for the width of the terrain model
	/// @return width of the model
	int getModelWidth();

	/// @brief Getter for the height of the data.
	/// @return the number of rows.
	int getDataHeight();

	/// @brief Getter for the height of the terrain model
	/// @return height of the model
	int getModelHeight();

	/// @brief Getter for the number of datapoints.
	/// @return columns * rows.
	int getElem();

	/// @brief Get the sampling rate of data for constructing the model.
	/// @return int value of the sample rate.
	int getSampleFactor();

	/// @brief Get the terrain scale.
	/// @return Return a float of the diff between max and min sample in the data.
	GLfloat getTerrainScale();

	/// @brief Get the texture id for the terrain height and normals.
	/// @return Returns the ID for the terrain data texture.
	GLuint getTextureID();

	GLuint getHeightBuffer();

	/// @brief Pointer to the terrain model
	///
	/// Used to get the model for drawing in the main program
	/// @return Model to draw.
	std::vector<Model*>* getModel();

	
	void initCompute();

	void runCompute();



};

#endif // READDATA_H
