/// @file program.cpp
/// @brief Implementations of functions in program.h

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

#include "program.h"
#include "Utilities.h"
#include "LoadTGA.h"
#include <vector>
#include <iostream>

Program::Program() {
	screenW = 600;
	screenH = 600;

	isRunning = true;

	TwTerminate();
}

Program::~Program() {}

int Program::exec() {
	if (!init()) return -1;

	SDL_Event Event;

	while (isRunning) {
		while (SDL_PollEvent(&Event)) handleEvent(Event);
		checkKeys();
		update();
		display();
	}

	clean();
	return 0;
}

void Program::clean() {
	TwTerminate();
	SDL_GL_DeleteContext(glcontext);
	SDL_Quit();
}

bool Program::init() {
	// SDL, glew and OpenGL init
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "Failed to initialise SDL: %s", SDL_GetError());
		return false;
	}
	screen = SDL_CreateWindow("TSBB11, Waterflow visualization (SDL)", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenW, screenH, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (screen == 0) {
		fprintf(stderr, "Failed to set Video Mode: %s", SDL_GetError());
		return false;
	}
	glcontext = SDL_GL_CreateContext(screen);
	glClearColor(0.1f, 0.7f, 0.1f, 1.0f);

	printError("After SDL init: ");

#ifdef __APPLE__
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);

	glViewport(0, 0, screenW, screenH);
	glMatrixMode(GL_PROJECTION);
	glOrtho(0, width, 0, height, -1, 1);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
#endif

#ifdef WIN32
	glewInit();
#endif

	dumpInfo();

	// Initial placement of camera.
	cam = new Camera(glm::vec3(0.0f,500.0f,0.0f), &screenW, &screenH);

	// Load terrain data
	dataHandler = new DataHandler("resources/output.min.asc");
	terrain = dataHandler->getModel();

	// Load and compile shaders.
	program = loadShaders("src/shaders/main.vert", "src/shaders/main.frag");
	skyshader = loadShaders("src/shaders/skyshader.vert", "src/shaders/skyshader.frag");

	// Initial one-time shader uploads.
	// Light information:
	glm::vec3 sunPos = { 0.58f, 0.58f, 0.58f }; // Since the sun is a directional source, this is the negative direction, not the position.
	bool sunIsDirectional = 1;
	float sunSpecularExponent = 50.0;
	glm::vec3 sunColor = { 1.0f, 1.0f, 1.0f };
	GLfloat sun_GLf[3] = { sunPos.x, sunPos.y, sunPos.z };
	glUseProgram(program);
	glUniform3fv(glGetUniformLocation(program, "lightSourcePos"), 1, sun_GLf);
	glUniform1i(glGetUniformLocation(program, "isDirectional"), sunIsDirectional);
	glUniform1fv(glGetUniformLocation(program, "specularExponent"), 1, &sunSpecularExponent);
	GLfloat sunColor_GLf[3] = { sunColor.x, sunColor.y, sunColor.z };
	glUniform3fv(glGetUniformLocation(program, "lightSourceColor"), 1, sunColor_GLf);

	/*Initialize AntTweakBar
	*/
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(screenW, screenH);

	myBar = TwNewBar("Test bar");
	TwAddVarRO(myBar, "CameraX", TW_TYPE_FLOAT, &cam->getPos()->x, "");
	TwAddVarRO(myBar, "CameraY", TW_TYPE_FLOAT, &cam->getPos()->y, "");
	TwAddVarRO(myBar, "CameraZ", TW_TYPE_FLOAT, &cam->getPos()->z, "");
	TwAddVarRO(myBar, "Height", TW_TYPE_FLOAT, &heightAtPos, "help= 'Shows terrain height at camera position' ");
	TwAddVarRW(myBar, "MovSpeed", TW_TYPE_FLOAT, cam->getSpeedPtr(), " min=0 max=10 step=0.01 group=Engine label='Movement speed' ");

	/* Initialize skycube */
	skycube = generateCube(2.0f);
	teapot = LoadModelPlus("resources/teapot.obj");

	// Creating cubemap texture
	glGenTextures(1, &tex_cube);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex_cube);

	TextureData texture1;
	memset(&texture1, 0, sizeof(texture1));
	TextureData texture2;
	memset(&texture2, 0, sizeof(texture2));
	TextureData texture3;
	memset(&texture3, 0, sizeof(texture3));
	TextureData texture4;
	memset(&texture4, 0, sizeof(texture4));
	TextureData texture5;
	memset(&texture5, 0, sizeof(texture5));
	TextureData texture6;
	memset(&texture6, 0, sizeof(texture6));

	if (!LoadTGATextureData("resources/Skycube/Xn.tga", &texture1)) return false;
	if (!LoadTGATextureData("resources/Skycube/Xp.tga", &texture2)) return false;
	if (!LoadTGATextureData("resources/Skycube/Yn.tga", &texture3)) return false;
	if (!LoadTGATextureData("resources/Skycube/Yp.tga", &texture4)) return false;
	if (!LoadTGATextureData("resources/Skycube/Zn.tga", &texture5)) return false;
	if (!LoadTGATextureData("resources/Skycube/Zp.tga", &texture6)) return false;

	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, texture1.width, texture1.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture1.imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, texture2.width, texture2.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture2.imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, texture3.width, texture3.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture3.imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, texture4.width, texture4.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture4.imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, texture5.width, texture5.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture5.imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, texture6.width, texture6.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture6.imageData);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	printError("After total init: ");

	return true;
}

void Program::update() {
	// Update the tweak bar
	heightAtPos = dataHandler->giveHeight(cam->getPos()->x, cam->getPos()->z);
}

void Program::display() {
	// Clear the screen.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// ====================== Draw skybox ===========================
	glUseProgram(skyshader);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	// ---Camera shader data---
	cam->uploadCamData(skyshader);

	glUniform1i(glGetUniformLocation(skyshader, "cube_texture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex_cube);

	DrawModel(skycube, skyshader, "in_Position", NULL, NULL);

	// ====================== Draw Terrain ==========================
	glUseProgram(program);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// ---Camera shader data---
	cam->uploadCamData(program);
	dataHandler->drawTerrain(program);

	// ====================== Draw AntBar ===========================
	TwDraw();

	printError("After display: ");

	SDL_GL_SwapWindow(screen);
}

// Handle events.
void Program::handleEvent(SDL_Event event) {
	switch (event.type) {
	case SDL_QUIT:
		exit(0);
		break;
	case SDL_KEYDOWN:
		handleKeypress(event);
		break;
	case SDL_WINDOWEVENT:
		switch (event.window.event) {
		case SDL_WINDOWEVENT_RESIZED:
			SDL_SetWindowSize(screen, event.window.data1, event.window.data2);
			SDL_GetWindowSize(screen, &screenW, &screenH);
			cam->updateVTP(&screenW, &screenH);
			break;
		}
		break;
	case SDL_MOUSEMOTION:
		handleMouse(event);
		break;
	case SDL_MOUSEBUTTONDOWN:
		TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_LEFT);
		handleMouse(event);
		break;
	case SDL_MOUSEBUTTONUP:
		TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_LEFT);
		handleMouse(event);
		break;
	default:
		break;
	}
}

// Handle keys
void Program::handleKeypress(SDL_Event event) {
	TwKeyPressed(event.key.keysym.sym, TW_KMOD_NONE);
	switch (event.key.keysym.sym) {
	case SDLK_ESCAPE:
		//case SDLK_q:
		exit(0);
		break;
	case SDLK_g:
		SDL_SetRelativeMouseMode(SDL_FALSE);
		break;
	case SDLK_h:
		SDL_SetRelativeMouseMode(SDL_TRUE);
		break;
	case SDLK_e:
		if (bar_vis == 1) {
			bar_vis = 0;
			TwDefine("myBar/visible=false");
		} else {
			bar_vis = 1;
			TwDefine("myBar/visible=true");
		} // mybar is displayed again
	default:
		break;
	}
}

void Program::handleMouse(SDL_Event event) {
	const Uint8 *state = SDL_GetKeyboardState(NULL);
	//When shift held camera doesn't move with mouse.
	if (!state[SDL_SCANCODE_LSHIFT]) {
		cam->changeLookAtPos(event.motion.xrel, event.motion.y);
	} else {
		TwMouseMotion(event.motion.x, event.motion.y);
	}
	/* Callback funciton for left mouse button. Retrieves x and y ((0, 0) is upper left corner from this function) of mouse.
	glReadPixels is used to retrieve Z-values from depth buffer. Here width-y is passed to comply with OpenGL implementation.
	glGetIntegerv retrievs values of Viewport matrix to pass to gluUnProject later. gluUnProject retrievs the original model
	coordinates from screen coordinates and Z-value. objY contains terrain height at clicked position after gluUnProject.
	*/
	if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) {

		float depth;
		int x;
		int y;
		SDL_GetMouseState(&x, &y);
		glReadPixels(x, screenW - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

		GLdouble objX = 0.0;
		GLdouble objY = 0.0;
		GLdouble objZ = 0.0;
		GLint viewport[4] = { 0, 0, 0, 0 };
		glGetIntegerv(GL_VIEWPORT, viewport);
		gluUnProject((GLdouble)x, (GLdouble)(screenW - y), (GLdouble)depth, glm::value_ptr((glm::dmat4)*cam->getWTV()), glm::value_ptr((glm::dmat4)*cam->getVTP()), viewport, &objX, &objY, &objZ);

		std::cout << "\nHeight at clicked pos (inverse coords): " << objY << std::endl;
		std::cout << "Height at clicked pos (from mapdata at (x,z)): " << dataHandler->giveHeight((GLfloat)objX, (GLfloat)objZ) << std::endl;
	}
}

void Program::checkKeys() {
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	if (keystate[SDL_SCANCODE_W]) {
		cam->forward(1.0f);
	} else if (keystate[SDL_SCANCODE_S]) {
		cam->forward(-1.0f);
	}
	if (keystate[SDL_SCANCODE_A]) {
		cam->strafe(1.0f);
	} else if (keystate[SDL_SCANCODE_D]) {
		cam->strafe(-1.0f);
	}
	if (keystate[SDL_SCANCODE_Q]) {
		cam->jump(1.0f);
	} else if (keystate[SDL_SCANCODE_E]) {
		cam->jump(-1.0f);
	}
}