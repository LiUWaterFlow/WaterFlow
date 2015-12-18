/// @file heightField.h
/// @brief Contains the CPU and GPU implementation of heightfield.

#ifndef HEIGHTFIELD_H
#define HEIGHTFIELD_H
#include "readData.h"
#include "glm.hpp"
#include "gtc/type_ptr.hpp"
#include <vector>
#include "xmlParsing.h"
#include "flowSource.h"

/// @class HeightField
/// @brief handles the heightfield simulation
class HeightField {
private:
	GLuint numVoxels; ///< Number of voxels in the voxelPositions.
	GLuint voxelShader; ///< Shader program.
	GLuint voxelBuffer; ///< Buffer for CPU voxels
	GLuint voxelVAO; ///< VAO for CPU voxels

	const static int width = 50; ///< Width used for CPU test case
	const static int height = 50; ///< Height used for CPU test case

	GLuint texWidth; ///< Width of the textures (buffers) used for the simulations
	GLuint texHeight; ///< Height of the textures (buffers) used for the simulations

	DataHandler* terr; ///< Handle to the terrain. (Datahandler).
	GLfloat u[width][height]; ///< The heightmap used for CPU testing
	GLfloat unew[width][height]; ///< Secondary heightmap used for CPU testing
	GLfloat v[width][height]; ///< Velocity field used for CPU testing.
	std::vector<GLuint>* voxelPositions; ///< array of voxels that are to be rendered (legacy).
	const static GLint samp = 1; ///< sample rate used for CPU testing.
	GLfloat totTime; ///< Time taken for the CPU testing.
	std::vector<Flood_Fill_data*> flood; ///< Vector containing the location info of the initial floodfills.

	/// @brief clamp (or clip) data between two values.
	///
	/// clips the int data n between the lower and upper value provided.
	int clip(int n, int lower, int upper);


	/// @brief clamp (or clip) data between two values.
	///
	/// clips the float data n between the lower and upper value provided.
	float clipf(GLfloat n, GLfloat lower, GLfloat upper);

	/// @brief Floodfills the simulation based on the values provided by the xml document.
	///
	/// This function floodfills the simulation based on the XML document provided.
	/// And is automatically called from initGPU.
	/// @see initGPU
	void initFloodFill(float* u);


	std::vector<FlowSource*> xmlFlow; ///< Flow sources
	float vol0; ///< Initial volume

	GLuint drawBuffers[4]; ///< buffers used for rendering (CPU)

	/// @brief updateSim
	///
	/// Runs the CPU simulation.
	/// @warning Legacy code left in for documentation purposes. This code is not guaranteed to work and have been moved to private
	/// for that reason.
	void updateSim(GLfloat);

	/// @brief Render the results from the CPU
	///
	/// @warning This function is not guaranteed to work as the simulation method has moved to the GPU
	/// and has been moved to private as not to be used by mistake.
	void render();
	/// @brief Initializes the necessary components for rendering the CPU tests.
	///
	/// @warning This function is not guaranteed to work as the simulation method has moved to the GPU
	/// and has been moved to private as not to be used by mistake.
	void initDraw();

	/// @brief Initializes the predefines test data for the CPU
	///
	/// @warning This function is not guaranteed to work as the simulation method has moved to the GPU
	/// and has been moved to private as not to be used by mistake.
	void initTest();
	/// @brief Initializes the predefines test data for the CPU
	///
	/// @warning This function is not guaranteed to work as the simulation method has moved to the GPU
	/// and has been moved to private as not to be used by mistake.
	void updateVoxelrender();
	/// @brief Draws all current voxels (CPU implementation).
	///
	/// @warning This function is not guaranteed to work as the simulation method has moved to the GPU
	/// and has been moved to private as not to be used by mistake.
	void drawVoxels(glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
	/// @brief Returns the voxels to be rendered's positions.
	///
	/// @warning This function is not guaranteed to work as the simulation method has moved to the GPU
	/// and has been moved to private as not to be used by mistake.
	std::vector<GLuint>* getVoxelPositions();

public:

	GLuint fieldProgram; ///< Shader used for simulation
	GLuint fieldShader; ///< Shader used for simulation
	GLuint addProgram; ///< Shader used for simulation
	GLuint addShader; ///< Shader used for simulation
	GLuint fieldBuffers[5]; ///< The buffers used for the GPU simulation

	/// @brief floodfills the specified point up to the height specified.
	///
	/// the array of floats u is the height that will be uploaded to the GPU.
	/// @see initFloodFill
	void floodFill(float* u, int x, int z, float height);

	/// @brief Measures the current volume of the water in the simulation
	///
	/// @warning This function is not properly implemented and doesn't handle added water.
	/// and has been known to give performance issues on certain systems.
	void measureVolume();

	/// @brief Constructore for the HeightField.
	///
	/// Takes a pointer to the datahandler for the terrain and the two vectors for flows and floodfill data.
	HeightField(DataHandler *t, std::vector<Flood_Fill_data*> FFDataIn, std::vector<FlowSource*> FlowsourcesIN);

	/// @brief Initializes the buffers and data necessary for the GPU simulation.
	void initGPU(float** heightArray, float** velocityArray);
	/// @brief Runs the simulation with the specified dt
	///
	///
	/// @note Must be initialized by initGPU
	/// @see initGPU
	void runSimGPU(GLfloat dt = 1.0f / 30.0f);

	/// @brief Modified getHeight for use with heightfield
	///
	/// The function clips the position to be in the intervall [0,width-1] for i
	/// and [0,height-1] for j.
	/// @return Returns ourHeight if the height at i,j (clipped) is lower than the terrain height.
	GLfloat getHeight(int i, int j, GLfloat ourHeight);
	
	/// @brief Saves the current simulation
	///
	/// Neccesary data for simulation (height and velocity) is saved to the input path
	/// @param file to save the data to
	void saveData(float** heightArray, float** velocityArray);
	
	
};
#endif
