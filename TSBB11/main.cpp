// The code below is based upon Ingemar Ragnemalm's code provided for the
// course TSBK03 at Linkï¿½ping University. The link to the original shell is
// http://www.ragnemalm.se/lightweight/psychteapot+MicroGlut-Windows-b1.zip
// which according to the web site was updated 2015-08-17.
//
// SDL functions written by Gustav Svensk, acquired with permissions from
// https://github.com/DrDante/TSBK03Project/ 2015-09-24. Some related code

// Notes:
// * Use glUniform1fv instead of glUniform1f, since glUniform1f has a bug under Linux.

#ifdef __APPLE__
#include <OpenGL/gl3.h>
// Uses framework Cocoa.
#endif
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"
#include <cstdlib>
#include <iostream>
#include "inc\controls.h"
#include "common\Windows\sdl2\SDL.h"
#include "common\glm\glm.hpp"
#include "common\glm\gtc/matrix_transform.hpp"
#include "common\glm\gtc/type_ptr.hpp"
#include "common\glm\gtx/transform.hpp"
#include "common\glm\gtx/transform2.hpp"
#include "common\glm\ext.hpp"
#include "common\glm\gtx/string_cast.hpp"
#include "common\custom\camera.h"
#include "common\SDL_util.h"

#ifndef NULL
#define NULL 0L
#endif

// Skärmstorlek
int width = 600;
int height = 600; // Defines instead?
float scl = 6;
#define DRAW_DISTANCE 3000.0

#define DISPLAY_TIMER 0
#define UPDATE_TIMER 1

#define SPEED 2

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

// Transformation matrices:
mat4 projMat, viewMat;

// Models:
Model *m;
Model *terrain;

// Textures:
TextureData ttex; // Terrain heightmap.

// References to shader programs:
GLuint program;

// Camera variables:
Camera cam;
// Matrices.
glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;

vec3 camPos = { 0, 20, 20 }; // p.
vec3 camLookAtPoint = { 0, 0, 0 }; // l.
vec3 camUp = { 0, 1, 0 }; // v.
vec3 camForward = camLookAtPoint - camPos; // s.

// Light information:
vec3 sunPos = { 0.58f, 0.58f, 0.58f }; // Since the sun is a directional source, this is the negative direction, not the position.
bool sunIsDirectional = 1;
float sunSpecularExponent = 50.0;
vec3 sunColor = { 1.0f, 1.0f, 1.0f };

// Some basic functions. These should already be moved to a separate source file in the dataset branch.
Model* GenerateTerrain(TextureData *tex, GLfloat terrainScale); // Generates a model given a height map (grayscale .tga file for now).
vec3 giveNormal(int x, int y, int z, GLfloat *vertexArray, GLuint *indexArray, int width, int height); // Returns the normal of a vertex.
GLfloat giveHeight(GLfloat x, GLfloat z, GLfloat *vertexArray, int width, int height); // Returns the height of a height map.

void init(void)
{
#ifdef WIN32
	glewInit();
#endif
	//initKeymapManager();
	dumpInfo();

	// GL inits.
	glClearColor(0.1, 0.1, 0.1, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_TRUE);

	// Load and compile shaders.
	program = loadShaders("shaders/main.vert", "shaders/main.frag");
	glUseProgram(program);
	// Initial placement of camera.
	cam = Camera(program, &viewMatrix);
	// ---Upload geometry to the GPU---
	LoadTGATextureData("resources/fft-terrain.tga", &ttex);
	terrain = GenerateTerrain(&ttex, 2);

	m = LoadModelPlus("resources/teapot.obj");
	// --------------------------------
	
	// Camera inits.
	//projMat = frustum(-0.5, 0.5, -0.5, 0.5, 1.0, DRAW_DISTANCE);
	//viewMat = lookAtv(camPos, camLookAtPoint, camUp);
	
	// Initial one-time shader uploads.
	glUniformMatrix4fv(glGetUniformLocation(program, "VTPMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	GLfloat sun_GLf[3] = { sunPos.x, sunPos.y, sunPos.z };
	glUniform3fv(glGetUniformLocation(program, "lightSourcePos"), 1, sun_GLf);
	glUniform1i(glGetUniformLocation(program, "isDirectional"), sunIsDirectional);
	glUniform1fv(glGetUniformLocation(program, "specularExponent"), 1, &sunSpecularExponent);
	GLfloat sunColor_GLf[3] = { sunColor.x, sunColor.y, sunColor.z };
	glUniform3fv(glGetUniformLocation(program, "lightSourceColor"), 1, sunColor_GLf);
}

void display(void)
{
	mat4 rot, trans, scale, total;

	// Time.
	//GLfloat t;
	//t = glutGet(GLUT_ELAPSED_TIME) / 100.0;

	// Clear the screen.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// ---Model-independent shader data---
	glUniformMatrix4fv(glGetUniformLocation(program, "WTVMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
	GLfloat camPos_GLf[3] = { camPos.x, camPos.y, camPos.z };
	glUniform3fv(glGetUniformLocation(program, "camPos"), 1, camPos_GLf);
	//glUniform1fv(glGetUniformLocation(program, "t"), 1, &t); 
	// ---Model-independent shader data---

	// ---Model transformations, rendering---
	// Terrain:
	trans = T(-100, -100, -100);
	total = trans;
	glUniformMatrix4fv(glGetUniformLocation(program, "MTWMatrix"), 1, GL_TRUE, total.m);
	DrawModel(terrain, program, "in_Position", "in_Normal", "in_TexCoord");

	// Teapot:
	trans = T(0, 0, 0);
	scale = S(0.5, 0.5, 0.5);
	total = Mult(trans, scale);
	glUniformMatrix4fv(glGetUniformLocation(program, "MTWMatrix"), 1, GL_TRUE, total.m);
	DrawModel(m, program, "in_Position", "in_Normal", "in_TexCoord");
	// --------------------------------------
	
	swap_buffers();
}

// Display timer. User made functions may NOT be called from here.
Uint32 display_timer(Uint32 interval, void* param)
{
	SDL_Event event;

	event.type = SDL_USEREVENT;
	event.user.code = DISPLAY_TIMER;
	event.user.data1 = 0;
	event.user.data2 = 0;

	SDL_PushEvent(&event);
	return interval;
}

Uint32 update_timer(Uint32 interval, void* param)
{
	SDL_Event event;

	event.type = SDL_USEREVENT;
	event.user.code = UPDATE_TIMER;
	event.user.data1 = (void*)(intptr_t)interval;
	event.user.data2 = 0;

	SDL_PushEvent(&event);
	return interval;
}

// Handle events.
void event_handler(SDL_Event event)
{
	switch (event.type){
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
			switch (event.window.event){
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
		default:
			break;
	}
}

// Handle user defined events.
void handle_userevent(SDL_Event event)
{
	switch (event.user.code){
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
void handle_keypress(SDL_Event event)
{
	switch (event.key.keysym.sym){
		case SDLK_ESCAPE:
		case SDLK_q:
			exit(0);
			break;
		case SDLK_g:
			SDL_SetRelativeMouseMode(SDL_FALSE);
			break;
		case SDLK_h:
			SDL_SetRelativeMouseMode(SDL_TRUE);
			break;
		default:
			break;
	}
}

void handle_mouse(SDL_Event event)
{
	get_window_size(&width, &height);
	cam.change_look_at_pos(event.motion.xrel, event.motion.y, width, height);
}

void check_keys()
{
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	if (keystate[SDL_SCANCODE_W]) {
		cam.forward(0.05*scl*SPEED);
	}
	else if (keystate[SDL_SCANCODE_S]) {
		cam.forward(-0.05*scl*SPEED);
	}
	if (keystate[SDL_SCANCODE_A]) {
		cam.strafe(0.05*scl*SPEED);
	}
	else if (keystate[SDL_SCANCODE_D]) {
		cam.strafe(-0.05*scl*SPEED);
	}
}

int main(int argc, char *argv[])
{
	init_SDL((const char*) "TSBB11, Waterflow visualization (SDL)", width, height);
	reshape(width, height, projectionMatrix);
	init();
	SDL_TimerID timer_id;
	timer_id = SDL_AddTimer(30, &display_timer, NULL);
	timer_id = SDL_AddTimer(10, &update_timer, NULL);
	if (timer_id == 0){
		std::cerr << "Error setting timer function: " << SDL_GetError() << std::endl;
	}
	set_event_handler(&event_handler);
	inf_loop();
	return 0;
}

// -----------------Ingemars hjälpfunktioner-----------------
void reshape(int w, int h, glm::mat4 &projectionMatrix)
{
	glViewport(0, 0, w, h);
	float ratio = (GLfloat)w / (GLfloat)h;
	projectionMatrix = glm::perspective(PI / 2, ratio, 1.0f, 1000.0f);
}
// ----------------------------------------------------------

Model* GenerateTerrain(TextureData *tex, GLfloat terrainScale)
{
	int vertexCount = tex->width * tex->height;
	int triangleCount = (tex->width - 1) * (tex->height - 1) * 2;
	int x, z;

	GLfloat *vertexArray = (GLfloat *)malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *normalArray = (GLfloat *)malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *texCoordArray = (GLfloat *)malloc(sizeof(GLfloat) * 2 * vertexCount);
	GLuint *indexArray = (GLuint *)malloc(sizeof(GLuint)* triangleCount * 3);

	vec3 tempNormal = { 0, 0, 0 };

	printf("bpp %d\n", tex->bpp);
	for (x = 0; x < tex->width; x++)
	{
		for (z = 0; z < tex->height; z++)
		{
			// Vertex array.
			vertexArray[(x + z * tex->width) * 3 + 0] = x / 1.0;
			vertexArray[(x + z * tex->width) * 3 + 1] = tex->imageData[(x + z * tex->width) * (tex->bpp / 8)] / terrainScale; // Terrain height.
			vertexArray[(x + z * tex->width) * 3 + 2] = z / 1.0;

			// Texture coordinates.
			texCoordArray[(x + z * tex->width) * 2 + 0] = x;
			texCoordArray[(x + z * tex->width) * 2 + 1] = z;
		}
	}

	for (x = 0; x < tex->width - 1; x++)
	{
		for (z = 0; z < tex->height - 1; z++)
		{
			// Triangle 1.
			indexArray[(x + z * (tex->width - 1)) * 6 + 0] = x + z * tex->width;
			indexArray[(x + z * (tex->width - 1)) * 6 + 1] = x + (z + 1) * tex->width;
			indexArray[(x + z * (tex->width - 1)) * 6 + 2] = x + 1 + z * tex->width;
			// Triangle 2.
			indexArray[(x + z * (tex->width - 1)) * 6 + 3] = x + 1 + z * tex->width;
			indexArray[(x + z * (tex->width - 1)) * 6 + 4] = x + (z + 1) * tex->width;
			indexArray[(x + z * (tex->width - 1)) * 6 + 5] = x + 1 + (z + 1) * tex->width;
		}
	}

	for (x = 0; x < tex->width; x++)
	{
		for (z = 0; z < tex->height; z++)
		{
			// Normal vectors.
			tempNormal = giveNormal(x, tex->imageData[(x + z * tex->width) * (tex->bpp / 8)] / 10.0, z, vertexArray, indexArray, tex->width, tex->height);
			normalArray[(x + z * tex->width) * 3 + 0] = -tempNormal.x;
			normalArray[(x + z * tex->width) * 3 + 1] = -tempNormal.y;
			normalArray[(x + z * tex->width) * 3 + 2] = -tempNormal.z;
		}
	}

	// End of terrain generation.

	// Create Model and upload to GPU.
	Model* model = LoadDataToModel(
		vertexArray,
		normalArray,
		texCoordArray,
		NULL,
		indexArray,
		vertexCount,
		triangleCount * 3);

	return model;
}

vec3 giveNormal(int x, int y, int z, GLfloat *vertexArray, GLuint *indexArray, int width, int height)
{
	vec3 vertex = { GLfloat(x), GLfloat(y), GLfloat(z) };
	vec3 normal = { 0, 1, 0 };

	vec3 normal1 = { 0, 0, 0 };
	vec3 normal2 = { 0, 0, 0 };
	vec3 normal3 = { 0, 0, 0 };
	vec3 normal4 = { 0, 0, 0 };
	vec3 normal5 = { 0, 0, 0 };
	vec3 normal6 = { 0, 0, 0 };

	if ((x > 1) && (z > 1) && (z < height - 2) && (x < width - 2))
	{
		vec3 tempVec1 = { vertexArray[indexArray[((x - 1) + (z - 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x - 1) + (z - 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x - 1) + (z - 1) * (width - 1)) * 6 + 0] * 3 + 2] };

		vec3 tempVec2 = { vertexArray[indexArray[((x)+(z - 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x)+(z - 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x)+(z - 1) * (width - 1)) * 6 + 0] * 3 + 2] };

		vec3 tempVec3 = { vertexArray[indexArray[((x - 1) + (z)* (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x - 1) + (z)* (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x - 1) + (z)* (width - 1)) * 6 + 0] * 3 + 2] };

		vec3 tempVec4 = { vertexArray[indexArray[((x + 1) + (z)* (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x + 1) + (z)* (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x + 1) + (z)* (width - 1)) * 6 + 0] * 3 + 2] };

		vec3 tempVec5 = { vertexArray[indexArray[((x + 1) + (z + 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x + 1) + (z + 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x + 1) + (z + 1) * (width - 1)) * 6 + 0] * 3 + 2] };

		vec3 tempVec6 = { vertexArray[indexArray[((x)+(z + 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x)+(z + 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x)+(z + 1) * (width - 1)) * 6 + 0] * 3 + 2] };


		normal1 = CrossProduct(VectorSub(tempVec1, vertex), VectorSub(tempVec2, vertex));
		normal2 = CrossProduct(VectorSub(tempVec3, vertex), VectorSub(tempVec1, vertex));
		vec3 weighted1 = Normalize(VectorAdd(Normalize(normal1), Normalize(normal2)));
		normal3 = CrossProduct(VectorSub(tempVec2, vertex), VectorSub(tempVec4, vertex));
		vec3 weighted2 = Normalize(normal3);
		normal4 = CrossProduct(VectorSub(tempVec4, vertex), VectorSub(tempVec5, vertex));
		normal5 = CrossProduct(VectorSub(tempVec5, vertex), VectorSub(tempVec6, vertex));
		vec3 weighted3 = Normalize(VectorAdd(Normalize(normal4), Normalize(normal5)));
		normal6 = CrossProduct(VectorSub(tempVec6, vertex), VectorSub(tempVec3, vertex));
		vec3 weighted4 = Normalize(normal6);

		normal = Normalize(VectorAdd(weighted1, VectorAdd(weighted2, VectorAdd(weighted3, weighted4))));

	}
	return normal;
}

GLfloat giveHeight(GLfloat x, GLfloat z, GLfloat *vertexArray, int width, int height)
{
	GLfloat yheight = 0;

	int vertX1 = floor(x);
	int vertZ1 = floor(z);

	int vertX2 = floor(x) + 1;
	int vertZ2 = floor(z) + 1;

	int vertX3 = 0;
	int vertZ3 = 0;

	if ((vertX1 > 1) && (vertZ1 > 1) && (vertX2 < height - 2) && (vertZ2 < width - 2))
	{

		GLfloat dist1 = vertX1 - x;
		GLfloat dist2 = vertZ1 - z;

		if (dist1 > dist2)
		{
			vertX3 = vertX1;
			vertZ3 = vertZ1 + 1;

		}
		else
		{
			vertX3 = vertX1 + 1;
			vertZ3 = vertZ1;

		}
		GLfloat vertY1 = vertexArray[(vertX1 + vertZ1 * width) * 3 + 1];

		GLfloat vertY2 = vertexArray[(vertX2 + vertZ2 * width) * 3 + 1];

		GLfloat vertY3 = vertexArray[(vertX3 + vertZ3 * width) * 3 + 1];

		vec3 p1 = { vertexArray[(vertX1 + vertZ1 * width) * 3 + 0], vertY1, vertexArray[(vertX1 + vertZ1 * width) * 3 + 2] };
		vec3 p2 = { vertexArray[(vertX2 + vertZ2 * width) * 3 + 0], vertY2, vertexArray[(vertX2 + vertZ2 * width) * 3 + 2] };
		vec3 p3 = { vertexArray[(vertX3 + vertZ3 * width) * 3 + 0], vertY3, vertexArray[(vertX3 + vertZ3 * width) * 3 + 2] };

		vec3 planeNormal = { 0, 0, 0 };

		// This if/else might not be making any difference whatsoever.
		if (dist1 > dist2)
		{
			planeNormal = Normalize(CrossProduct(VectorSub(p2, p1), VectorSub(p3, p1)));
		}
		else
		{
			planeNormal = Normalize(CrossProduct(VectorSub(p3, p1), VectorSub(p2, p1)));
		}

		GLfloat D;
		D = DotProduct(planeNormal, p1);

		yheight = (D - planeNormal.x*x - planeNormal.z*z) / planeNormal.y;
	}
	return yheight;
}