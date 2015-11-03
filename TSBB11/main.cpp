// The code below is based upon Ingemar Ragnemalm's code provided for the
// course TSBK03 at Link�ping University. The link to the original shell is
// http://www.ragnemalm.se/lightweight/psychteapot+MicroGlut-Windows-b1.zip
// which according to the web site was updated 2015-08-17.
//
// SDL functions written by Gustav Svensk, acquired with permissions from
// https://github.com/DrDante/TSBK03Project/ 2015-09-24. Some related code

// Notes:
// * Use glUniform1fv instead of glUniform1f, since glUniform1f has a bug under Linux.

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>
#else
#ifdef  __linux__
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <SDL2/SDL.h>


#else
#include "glew.h"
#include "Windows/sdl2/SDL.h"
#endif
#endif

#include <cstdlib>
#include <iostream>
#include "AntTweakBar.h"
#include "GL_utilities.h"
#include "loadobj.h"
#include "LoadTGA.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "gtx/transform.hpp"
#include "gtx/transform2.hpp"
#include "ext.hpp"
#include "gtx/string_cast.hpp"
#include "SDL_util.h"
#include "camera.h"
#include "readData.h"

#ifndef NULL
#define NULL 0L
#endif

// Screensize
int width = 600;
int height = 600; // Defines instead?
float scl = 6;

#define DRAW_DISTANCE 10000.0

#define DISPLAY_TIMER 0
#define UPDATE_TIMER 1

#define SPEED 30.0f

#define PI 3.14159265358979323846f

// SDL functions
void handle_keypress(SDL_Event event);
void handle_mouse(SDL_Event event);
static void event_handler(SDL_Event event);
void handle_userevent(SDL_Event event);
void check_keys();
// -------------

void reshape(int w, int h, glm::mat4 &projectionMatrix);
// -------------------------------------------------------------
//AntTweakBar variabels
int bar_vis;
TwBar *myBar;
float height_at_pos;
// Transformation matrices:
glm::mat4 projMat, viewMat;

// Models:
std::vector <Model*>* terrain;

glm::mat4 rot, trans, scale, total;

// Datahandler for terrain data
DataHandler* dataHandler;

// References to shader programs:
GLuint program;

// Camera variables:
Camera cam;
// Matrices.
glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;

// Light information:
glm::vec3 sunPos = { 0.58f, 0.58f, 0.58f }; // Since the sun is a directional source, this is the negative direction, not the position.
bool sunIsDirectional = 1;
float sunSpecularExponent = 50.0;
glm::vec3 sunColor = { 1.0f, 1.0f, 1.0f };

void TW_CALL Callback(void * clientData) {
	std::cout << "TW button pressed" << std::endl;
}

void init(void) {
#ifdef WIN32
	glewInit();
#endif
	//initKeymapManager();
	dumpInfo();

	// GL inits.
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_TRUE);

	// Initial placement of camera.
	cam = Camera(program, &viewMatrix);

	// Load terrain data
	dataHandler = new DataHandler("resources/output.min.asc");
	terrain = dataHandler->getModel();


	// ---Model transformations, rendering---
	// Terrain:
	scale = glm::scale(glm::vec3(dataHandler->getDataWidth(),
		dataHandler->getTerrainScale(),
		dataHandler->getDataHeight()));
	total = scale;


	// Load and compile shaders.
	program = loadShaders("src/shaders/main.vert", "src/shaders/main.frag");
	glUseProgram(program);

	// Initial one-time shader uploads.
	glUniformMatrix4fv(glGetUniformLocation(program, "VTPMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(glGetUniformLocation(program, "MTWMatrix"), 1, GL_FALSE, glm::value_ptr(total));
	GLfloat sun_GLf[3] = { sunPos.x, sunPos.y, sunPos.z };
	glUniform3fv(glGetUniformLocation(program, "lightSourcePos"), 1, sun_GLf);
	glUniform1i(glGetUniformLocation(program, "isDirectional"), sunIsDirectional);
	glUniform1fv(glGetUniformLocation(program, "specularExponent"), 1, &sunSpecularExponent);
	GLfloat sunColor_GLf[3] = { sunColor.x, sunColor.y, sunColor.z };
	glUniform3fv(glGetUniformLocation(program, "lightSourceColor"), 1, sunColor_GLf);

	/*Initialize AntTweakBar
	*/
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(width, height);


	myBar = TwNewBar("Test bar");
	TwAddVarRO(myBar, "CameraX", TW_TYPE_FLOAT, &cam.position.x, "");
	TwAddVarRO(myBar, "CameraY", TW_TYPE_FLOAT, &cam.position.y, "");
	TwAddVarRO(myBar, "CameraZ", TW_TYPE_FLOAT, &cam.position.z, "");
	TwAddVarRO(myBar, "Height", TW_TYPE_FLOAT, &height_at_pos, "help= 'Shows terrain height at camera position' ");
	TwAddVarRW(myBar, "MovSpeed", TW_TYPE_FLOAT, &scl, " min=0 max=10 step=0.01 group=Engine label='Movement speed' ");

	TwAddButton(myBar, "Run", Callback, NULL, " label='Run Forest' ");

}

void display(void) {

	glUseProgram(program);

	// Clear the screen.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// ---Camera shader data---
	glUniformMatrix4fv(glGetUniformLocation(program, "WTVMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
	GLfloat camPos_GLf[3] = { cam.position.x, cam.position.y, cam.position.z };
	glUniform3fv(glGetUniformLocation(program, "camPos"), 1, camPos_GLf);


	// precalculate the inverse since it is a very large model.
	glm::mat3 inverseNormalMatrixTrans = glm::transpose(glm::inverse(glm::mat3(total)));
	glUniformMatrix3fv(glGetUniformLocation(program, "iNormalMatrixTrans"), 1, GL_FALSE, glm::value_ptr(inverseNormalMatrixTrans));


	for (GLuint i = 0; i < terrain->size(); i++) {
		DrawModel(terrain->at(i), program, "in_Position", "in_Normal", "in_TexCoord");
	}
	// --------------------------------------

	/*Draw the tweak bars
	*/

	height_at_pos = dataHandler->getCoord((int)cam.position.x, (int)cam.position.z); // <- should be giveHeight when merged to master
	TwDraw();
	/*
		while( SDL_PollEvent(&event) ) // process events
		{
		// send event to AntTweakBar
		handled = TwEventSDL(&event, SDL_MAJOR_VERSION, SDL_MINOR_VERSION);
		}
		*/
	swap_buffers();
}

// Display timer. User made functions may NOT be called from here.
Uint32 display_timer(Uint32 interval, void* param) {
	SDL_Event event;

	event.type = SDL_USEREVENT;
	event.user.code = DISPLAY_TIMER;
	event.user.data1 = 0;
	event.user.data2 = 0;

	SDL_PushEvent(&event);
	return interval;
}

Uint32 update_timer(Uint32 interval, void* param) {
	SDL_Event event;

	event.type = SDL_USEREVENT;
	event.user.code = UPDATE_TIMER;
	event.user.data1 = (void*)(intptr_t)interval;
	event.user.data2 = 0;

	SDL_PushEvent(&event);
	return interval;
}

// Handle events.
void event_handler(SDL_Event event) {
	//int handled;
	//handled = TwEventSDL(&event, SDL_MAJOR_VERSION, SDL_MINOR_VERSION);
	switch (event.type) {
	case SDL_USEREVENT:
		handle_userevent(event);
		break;
	case SDL_QUIT:
		exit(0);
		break;
	case SDL_KEYDOWN:
		handle_keypress(event);
		break;
	case SDL_WINDOWEVENT:
		switch (event.window.event) {
		case SDL_WINDOWEVENT_RESIZED:
			get_window_size(&width, &height);
			resize_window(event);
			reshape(width, height, projectionMatrix);
			break;
		}
		break;
	case SDL_MOUSEMOTION:
		handle_mouse(event);
		break;
	case SDL_MOUSEBUTTONDOWN:
		TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_LEFT);
		handle_mouse(event);
		break;
	case SDL_MOUSEBUTTONUP:
		TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_LEFT);
		handle_mouse(event);
		break;
	default:
		break;
	}
}

// Handle user defined events.
void handle_userevent(SDL_Event event) {
	switch (event.user.code) {
	case (int)DISPLAY_TIMER:
		display();
		break;
	case (int)UPDATE_TIMER:
		check_keys();
		break;
	default:
		break;
	}
}

// Handle keys
void handle_keypress(SDL_Event event) {
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
	case SDLK_l:
		std::cout << "Height: " << dataHandler->giveHeight(cam.position.x, cam.position.z) << std::endl;
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

void handle_mouse(SDL_Event event) {
	get_window_size(&width, &height);
	const Uint8 *state = SDL_GetKeyboardState(NULL);
	//When shift held camera doesn't move with mouse.
	if (!state[SDL_SCANCODE_LSHIFT]) {
		cam.change_look_at_pos(event.motion.xrel, event.motion.y, width, height);
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
		glReadPixels(x, width - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

		GLdouble objX = 0.0;
		GLdouble objY = 0.0;
		GLdouble objZ = 0.0;
		GLint viewport[4] = { 0, 0, 0, 0 };
		glGetIntegerv(GL_VIEWPORT, viewport);
		gluUnProject((GLdouble)x, (GLdouble)(width - y), (GLdouble)depth, glm::value_ptr((glm::dmat4)(viewMatrix)), glm::value_ptr((glm::dmat4)projectionMatrix), viewport, &objX, &objY, &objZ);

		std::cout << "\nHeight at clicked pos (inverse coords): " << objY << std::endl;
		std::cout << "Height at clicked pos (from mapdata at (x,z)): " << dataHandler->giveHeight(objX, objZ) << std::endl;
	}
}

void check_keys() {
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	if (keystate[SDL_SCANCODE_W]) {
		cam.forward(0.05f*scl*SPEED);
	} else if (keystate[SDL_SCANCODE_S]) {
		cam.forward(-0.05f*scl*SPEED);
	}
	if (keystate[SDL_SCANCODE_A]) {
		cam.strafe(0.05f*scl*SPEED);
	} else if (keystate[SDL_SCANCODE_D]) {
		cam.strafe(-0.05f*scl*SPEED);
	}
}


// -----------------Ingemars hj�lpfunktioner-----------------
void reshape(int w, int h, glm::mat4 &projectionMatrix) {
	glViewport(0, 0, w, h);
	float ratio = (GLfloat)w / (GLfloat)h;
	projectionMatrix = glm::perspective(PI / 2, ratio, 1.0f, 10000.0f);
}
// ----------------------------------------------------------

int main(int argc, char *argv[]) {
	init_SDL((const char*) "TSBB11, Waterflow visualization (SDL)", width, height);

	reshape(width, height, projectionMatrix);
	glEnableClientState(GL_VERTEX_ARRAY);
	init();

	SDL_TimerID timer_id1, timer_id2;
	timer_id1 = SDL_AddTimer(30, &display_timer, NULL);
	timer_id2 = SDL_AddTimer(30, &update_timer, NULL);
	if (timer_id1 == 0 || timer_id2 == 0) {
		std::cerr << "Error setting timer function: " << SDL_GetError() << std::endl;
	}

	set_event_handler(&event_handler);
	inf_loop();
	TwTerminate();
	return 0;
}
