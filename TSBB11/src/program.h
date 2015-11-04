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

#include "AntTweakBar.h"
#include "glm.hpp"

#include <vector>

class Program {
private:
	SDL_Window *screen;
	SDL_GLContext glcontext;

	int screenW;
	int screenH;

	bool isRunning;

	//AntTweakBar variabels
	int bar_vis;
	TwBar *myBar;
	float heightAtPos;
	
	// Models:
	std::vector <Model*>* terrain;
	Model* skycube;
	Model* teapot;

	// Datahandler for terrain data
	DataHandler* dataHandler;

	// References to shader programs:
	GLuint program;
	GLuint skyshader;
	GLuint tex_cube;

	// Camera variables:
	Camera* cam;

public:
	int exec();

	bool init();
	
	void handleEvent(SDL_Event event);
	void handleKeypress(SDL_Event event);
	void handleMouse(SDL_Event event);
	void checkKeys();

	void update();
	void display();

	void clean();

	Program();
	~Program();
};

#endif // PROGRAM_H