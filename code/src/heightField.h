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
/// @brief handles the heightfield simulation. Members marked with (CPU), were used during the CPU trials and are left in as documentation but should be viewed as deprecated.
class HeightField {
private:
	GLuint numVoxels; ///< Number of voxels in the voxelPositions (CPU)
	GLuint voxelShader; ///< Shader program (CPU)
	GLuint voxelBuffer; ///< Buffer for voxels (CPU)
	GLuint voxelVAO; ///< VAO for voxels (CPU)

	const static int width = 50; ///< Width used for CPU test case (CPU)
	const static int height = 50; ///< Height used for CPU test case (CPU)

	GLfloat u[width][height]; ///< The heightmap used for CPU testing (CPU)
	GLfloat unew[width][height]; ///< Secondary heightmap used for CPU testing (CPU)
	GLfloat v[width][height]; ///< Velocity field used for CPU testing. (CPU)
	std::vector<GLuint>* voxelPositions; ///< array of voxels that are to be rendered (CPU).

	const static GLint samp = 1; ///< sample rate used for CPU testing. (CPU)
	GLfloat totTime; ///< Time taken for the CPU testing. (CPU)

	GLuint texWidth; ///< Width of the textures (buffers) used for the simulations
	GLuint texHeight; ///< Height of the textures (buffers) used for the simulations

	DataHandler* terr; ///< Handle to the terrain. (Datahandler).
	
	std::vector<Flood_Fill_data*> flood; ///< Vector containing the location info of the initial floodfills.

	/// @brief clamp (or clip) data between two values.
	///
	/// clips the int data n between the lower and upper values provided.
	int clip(int n, int lower, int upper);


	/// @brief clamp (or clip) data between two values.
	///
	/// clips the float data n between the lower and upper values provided.
	float clipf(GLfloat n, GLfloat lower, GLfloat upper);

	/// @brief Floodfills the simulation based on the values provided by the xml document.
	///
	/// This function floodfills the simulation based on the XML document provided.
	/// And is automatically called from initGPU.
	/// @see initGPU
	/// @note has been known to be slow on certain configurations of hardware especially in debug mode.
	void initFloodFill(float* u);


	std::vector<FlowSource*> xmlFlow; ///< Flow sources
	float vol0; ///< Initial volume

	GLuint drawBuffers[4]; ///< buffers used for rendering (CPU)

	/// @brief updateSim (CPU)
	///
	/// Runs the CPU simulation.
	/// @warning Legacy code left in for documentation purposes. This code is not guaranteed to work and have been moved to private
	/// for that reason.
	void updateSim(GLfloat);

	/// @brief Render the results from the CPU (CPU)
	///
	/// @warning This function is not guaranteed to work as the simulation method has moved to the GPU
	/// and has been moved to private as not to be used by mistake.
	void render();

	/// @brief Initializes the necessary components for rendering the CPU tests. (CPU)
	///
	/// @warning This function is not guaranteed to work as the simulation method has moved to the GPU
	/// and has been moved to private as not to be used by mistake.
	void initDraw();

	/// @brief Initializes the predefines test data for the CPU (CPU)
	///
	/// @warning This function is not guaranteed to work as the simulation method has moved to the GPU
	/// and has been moved to private as not to be used by mistake.
	void initTest();

	/// @brief Initializes the predefined test data for the CPU (CPU)
	///
	/// @warning This function is not guaranteed to work as the simulation method has moved to the GPU
	/// and has been moved to private as not to be used by mistake.
	void updateVoxelrender();

	/// @brief Draws all current voxels (CPU).
	///
	/// @warning This function is not guaranteed to work as the simulation method has moved to the GPU
	/// and has been moved to private as not to be used by mistake.
	void drawVoxels(glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
	
	/// @brief Returns the voxels to be rendered's positions. (CPU)
	///
	/// @warning This function is not guaranteed to work as the simulation method has moved to the GPU
	/// and has been moved to private as not to be used by mistake.
	std::vector<GLuint>* getVoxelPositions();


	GLuint fieldProgram; ///< Shader reference to fieldShader.comp which is the main simulation shader.
	GLuint addProgram; ///< Shader reference to addShader.comp which adds flows to the simulation.
	

public:

	GLuint fieldBuffers[5]; ///< The buffers used for the GPU simulation of heightfield

	/// @brief floodfills the specified point up to the height specified.
	///
	/// the array of floats u is the height that will be uploaded to the GPU.
	/// @note has been known to be slow on certain hardware/software configurations in debug mode.
	/// @see initFloodFill
	void floodFill(float* u, int x, int z, float height);

	/// @brief Measures the current volume of the water in the simulation
	///
	/// @warning This function is not properly implemented and doesn't handle added water.
	/// and has been known to give performance issues on certain systems. But is left in the project as documentation.
	void measureVolume();

	/// @brief Constructor for the HeightField.
	///
	/// Takes a pointer to the datahandler for the terrain and the two vectors for flows and floodfill data.
	/// The sequence below is how to properly setup a heightfield
	/// @code{.cpp}
	///	// Load previous simulation
	/// heightData = new FileHandler(dataHandler->getDataWidth(), dataHandler->getDataHeight());
	/// velocityData = new FileHandler(dataHandler->getDataWidth(), dataHandler->getDataHeight());
	///
	/// heightData->LoadData(init_data->height_load_path.c_str());
	/// velocityData->LoadData(init_data->velocity_load_path.c_str());
	/// // Initialize water simulation
	///	hf = new HeightField(dataHandler, init_data->FFData, init_data->Flowsources);
	///	hf->initGPU(heightData->GetArray(), velocityData->GetArray());
	///
	/// @endcode
	/// @see fileHandler::GetArray
	
	HeightField(DataHandler *t, std::vector<Flood_Fill_data*> FFDataIn, std::vector<FlowSource*> FlowsourcesIN);

	/// @brief Initializes the buffers and data necessary for the GPU simulation.
	///
	/// @param heightArray A pointer to the raw memory block which the fileHandler will use to load the simulation state
	/// @param velocityArray A pointer to the raw memory block which the fileHandler will use to load the simulation state
	/// 
	/// @see HeightField
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
	/// @code{.cpp}
	/// hf->saveData(heightData->GetArray(),velocityData->GetArray()); // Send pointers to raw memory blocks where the data should be saved
	/// heightData->SaveData(init_data->height_save_path.c_str()); //call SaveData with a filepath string. 
	/// velocityData->SaveData(init_data->velocity_save_path.c_str());
	/// @endcode
	/// Necessary data for simulation (height and velocity) is saved to the input paths specified by XML.
	/// @param heightArray pointer to raw memory where the data will be saved.
	/// @param velocityArray pointer to raw memory where the data will be saved.
	void saveData(float** heightArray, float** velocityArray);
};
#endif
