/// @file program.h
/// @brief Contains the main structure fuctions

#ifndef PROGRAM_H
#define PROGRAM_H

#ifdef __APPLE__
#	include <OpenGL/gl3.h>
#	include <SDL2/SDL.h>
#else
#	ifdef  __linux__
#		define GL_GLEXT_PROTOTYPES
#		include <GL/gl.h>
#		include <GL/glu.h>
#		include <GL/glx.h>
#		include <GL/glext.h>
#		include <SDL2/SDL.h>
#	else
#		include "glew.h"
#		include "Windows/sdl2/SDL.h"
#		include "SDL_image.h"
#		include "sdlTexture.h"
#	endif
#endif

#include "shallowGPU.h"
#include "camera.h"
#include "readData.h"
#include "myDrawable.h"
#include "voxel.h"
#include "heightField.h"
#include "xmlParsing.h"

#include "AntTweakBar.h"

/// @class Program
/// @brief Main structure of the program
///
/// This is where the broad strokes of the program should be outlined.
/// Contains the main loop of the program and funtions for the different phases.
class Program {
private:
	// Screen stuff
	SDL_Window *screen;			///< Handle for the SDL window
	SDL_GLContext glcontext;	///< SDL openGL context

	int screenW; ///< Screen width
	int screenH; ///< Screen height

	// States
	bool isRunning;		///< True as long as the program is running, set to false to quit
	bool mouseHidden;	///< Bool to check if the mouse is hidden or not
	bool depthWater;

	// Time variables
	GLfloat currentTime;	///< Total time since program started
	GLfloat deltaTime;		///< Time for last frame
	GLfloat FPS;			///< Current frames per second (based on deltaTime)

	GLfloat dtSim;

	//AntTweakBar variabels
	TwBar *antBar;				///< Handle for AntTweakBar GUI window
	float heightAtPos;			///< Terrain height at the comera position
	float heightAtClickData;	///< Terrain height at the clicked position
	float heighAtClickProj;		///< Terrain/water height at the clicked position
	double objX;				///< Terrain x coordinate at clicked position
	double objZ;				///< Terrain z coordinate at clicked position

	// myDrawables:
	myDrawable *skycube;		///< Skycube model
	myDrawable *terrain;		///< Terrain model
	myDrawable *waterTerrain;	///< Water model

	// Datahandler for terrain data
	DataHandler* dataHandler;	///< Holds data about the terrain

	//Voxgrid
	Voxelgrid* voxs;
	//HeightField
	HeightField* hf;
	//ShallowWater
	ShallowGPU* sgpu;

	// References to shader programs:
	GLuint terrainshader;		///< Shaders used for rendering terrain
	GLuint skyshader;			///< Shaders used for rendering the skybox
	GLuint watershader;			///< Shaders used for rendering the water
	GLuint depthshader;			///< Shaders used for rendering a depthmap of the water
	GLuint shallowwatershader;	///< Shaders used for rendering water in the shallow water simulation

	// Camera variables:
	Camera* cam; ///< Camera for moving in 3D

	GLuint simCase; ///< Which simulation is to be run.
	
	bool sim; ///<Determines if the simulation is paused or not.

public:
	/// @brief Simple constructor that selects which type of simulation to use
	/// @param simCase Selected simulation method (1 = heightfield, 2 = shallowWater)
	Program(GLuint simCase);

	/// @brief Simple destructor that just cleans up.
	~Program();

	/// @brief Main entry for running the program.
	///
	/// Contains the main loop for the program and the structure of which order the program phases comes in.
	/// @return Returns -1 if the init did not run correctly
	/// @see init()
	int exec();
	
	/// @brief All one time initializations for the program is done here.
	///
	/// The openGL context is created and all objects should be created and initialized here. 
	/// If the init does not return true the program exits. Currently this only happens if the 
	/// OpenGL context fails to initialize in some way.
	bool init();
	
	/// @brief Takes care of all events since last main loop iteration
	///
	/// Looks at all events and either takes care of the more uncommon ones like 
	/// screen events and quit events. Passes the other events along to the appropriate handler.
	/// @param event An event to be processed
	/// @see handleKeypress
	/// @see handleMouseMove
	void handleEvent(SDL_Event* event);

	/// @brief Handles Keyboard presses that should only happen once
	///
	/// Define events here when a single event should be generated from a keyboard press.
	/// For held down key events look at checkKeys() instead.
	/// @param event An event to be processed
	void handleKeypress(SDL_Event* event);

	/// @brief Handles movements of the mouse
	///
	/// Rotates the camera if the mouse pointer is hidden otherwise it just passes the movements to AntTweakBar
	/// so that it is possible to interact with the GUI.
	/// @param event An event to be processed
	/// @see checkKeys
	void handleMouseMove(SDL_Event* event);

	/// @brief Get data from mouse click
	///
	/// Callback funciton for left mouse button. Retrieves x and y ((0, 0) is upper left corner from this function) of mouse.
	/// glReadPixels is used to retrieve Z-values from depth buffer. Here width-y is passed to comply with OpenGL implementation.
	/// glGetIntegerv retrievs values of Viewport matrix to pass to gluUnProject later. gluUnProject retrievs the original model
	/// coordinates from screen coordinates and Z-value. objY contains terrain height at clicked position after gluUnProject.
	/// @note Left shift need to be pressed to collect data
	/// @param event An event to be processed
	void handleMouseButton(SDL_Event* event);

	/// @brief Checks which keys are held down each iteration
	///
	/// This is for events that requires buttons to be held down for longer periods of time.
	/// Currently used mainly for moving the camera around in the 3D environment. For single event
	/// keypresses use handleKeypress()
	/// @see handleKeypress
	void checkKeys();

	/// @brief Updates the time
	///
	/// Calculates the last frame time and updates the FPS and current time. This is not part of update
	/// since we want the same time delta for events as for the update.
	void timeUpdate();

	/// @brief Updates the simulation and camera
	///
	/// This is where the simulation is updated currently. After the simulation the model for the water
	/// is updated.
	void update();

	/// @brief All rendering commands are sent here
	///
	/// Sends the rendering commands to all drawables and uploads the camera uniforms to the programs.
	void display();

	/// @brief Makes sure the program exits cleanly
	///
	/// Closes the SDL OpenGL context correctly and closes the AntTweakBar.
	void clean();

	int testVoxels();
};

#endif // PROGRAM_H
