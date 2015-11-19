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
#	endif
#endif

#include "camera.h"
#include "readData.h"
#include "myDrawable.h"
#include "voxel.h"
#include "heightField.h"

#include "AntTweakBar.h"

class Program {
private:
	SDL_Window *screen;
	SDL_GLContext glcontext;

	int screenW, screenH;

	bool isRunning;

	bool mouseHidden;

	// Time variables
	GLfloat currentTime;
	GLfloat deltaTime;
	GLfloat FPS;

	//AntTweakBar variabels
	TwBar *antBar;
	float heightAtPos, heightAtClickData, heighAtClickProj;
	double objX, objZ;

	// myDrawables:
	myDrawable *skycube;
	Terrain  *terrain;

	// Datahandler for terrain data
	DataHandler* dataHandler;

	//Voxgrid
	Voxelgrid* voxs;
	//HeightField
	HeightField* hf;

	// References to shader programs:
	GLuint terrainshader, skyshader;

	// Camera variables:
	Camera* cam;

public:

	int exec();

	int testVoxels();

	bool init();

	void handleEvent(SDL_Event* event);
	void handleKeypress(SDL_Event* event);
	void handleMouseMove(SDL_Event* event);

	/// @brief Get data from mouse click
	///
	/// Callback funciton for left mouse button. Retrieves x and y ((0, 0) is upper left corner from this function) of mouse.
	/// glReadPixels is used to retrieve Z-values from depth buffer. Here width-y is passed to comply with OpenGL implementation.
	/// glGetIntegerv retrievs values of Viewport matrix to pass to gluUnProject later. gluUnProject retrievs the original model
	/// coordinates from screen coordinates and Z-value. objY contains terrain height at clicked position after gluUnProject.
	/// @note Left shift need to be pressed to collect data
	void handleMouseButton(SDL_Event* event);
	void checkKeys();

	void timeUpdate();
	void update();
	void display();

	void clean();

	Program();
	~Program();
};

#endif // PROGRAM_H
