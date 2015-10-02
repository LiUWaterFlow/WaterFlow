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
// Uses framework Cocoa.
#endif
#include <cstdlib>
#include <iostream>
#include "GL_utilities.h"
#include "loadobj.h"
#include "LoadTGA.h"
#include "sdl2/SDL.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "gtx/transform.hpp"
#include "gtx/transform2.hpp"
#include "ext.hpp"
#include "gtx/string_cast.hpp"
#include "SDL_util.h"
#include "camera.h"
//#include "readData.h"
#include "voxel.h"

#ifndef NULL
#define NULL 0L
#endif

// Sk�rmstorlek
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
glm::mat4 projMat, viewMat;

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

glm::vec3 camPos = { 0, 20, 20 }; // p.
glm::vec3 camLookAtPoint = { 0, 0, 0 }; // l.
glm::vec3 camUp = { 0, 1, 0 }; // v.
glm::vec3 camForward = camLookAtPoint - camPos; // s.

// Light information:
glm::vec3 sunPos = { 0.58f, 0.58f, 0.58f }; // Since the sun is a directional source, this is the negative direction, not the position.
bool sunIsDirectional = 1;
float sunSpecularExponent = 50.0;
glm::vec3 sunColor = { 1.0f, 1.0f, 1.0f };

// Some basic functions. These should already be moved to a separate source file in the dataset branch.
//Model* GenerateTerrain(TextureData *tex, GLfloat terrainScale); // Generates a model given a height map (grayscale .tga file for now).
//glm::vec3 giveNormal(int x, int y, int z, GLfloat *vertexArray, GLuint *indexArray, int width, int height); // Returns the normal of a vertex.
//GLfloat giveHeight(GLfloat x, GLfloat z, GLfloat *vertexArray, int width, int height); // Returns the height of a height map.

// ====== Needed for Normalized Convolution =======

GLfloat square[] = {-1, -1, 0,
					-1, 1, 0,
					1, 1, 0,
					1, -1, 0 };
GLfloat squareTexCoord[] = {0, 0,
							0, 1,
							1, 1,
							1, 0 };
GLuint squareIndices[] = { 0, 1, 2, 0, 2, 3 };

Model* squareModel;
FBOstruct *fbo1, *fbo2, *fbo3;
GLuint plaintextureshader = 0, filtershader = 0, confidenceshader = 0, combineshader = 0;
DataHandler* dataHandler;
Voxelgrid* grid;
int tW, tH;

void loadNormConvModel();
void performNormConv();

// ============================================

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

	// Initial placement of camera.
	cam = Camera(program, &viewMatrix);
	// ---Upload geometry to the GPU---
	//LoadTGATextureData("resources/fft-terrain.tga", &ttex);

	m = LoadModelPlus("resources/teapot.obj");

	//Construct the voxelgrid of size x,y,z (for now 50,50,50 to not slow down
	//the rest of the program




	dataHandler = new DataHandler("resources/output.min.asc", 500.0f);

	terrain = dataHandler->datamodel;

	grid = new Voxelgrid(dataHandler);
	grid->setVoxel(500,500,500,true, 1, 1);


	plaintextureshader = loadShaders("src/shaders/plaintextureshader.vert", "src/shaders/plaintextureshader.frag");  // puts texture on teapot
	filtershader = loadShaders("src/shaders/plaintextureshader.vert", "src/shaders/filtershader.frag");
	confidenceshader = loadShaders("src/shaders/plaintextureshader.vert", "src/shaders/confidenceshader.frag");
	combineshader = loadShaders("src/shaders/plaintextureshader.vert", "src/shaders/combineshader.frag");

	tW = dataHandler->getWidth();
	tH = dataHandler->getHeight();

	fbo1 = initFBO3(tW, tH, NULL);
	fbo2 = initFBO3(tW, tH, NULL);
	fbo3 = initFBO3(tW, tH, dataHandler->getData());

	squareModel = LoadDataToModel(square, NULL, squareTexCoord, NULL, squareIndices, 4, 6);


	program = loadShaders("src/shaders/main.vert", "src/shaders/main.frag");
	glUseProgram(program);

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
	glm::mat4 rot, trans, scale, total;

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
	glm::vec3 terrainTrans = glm::vec3(-100, -100, -100);
	trans = glm::translate(terrainTrans);
	total = trans;
	glUniformMatrix4fv(glGetUniformLocation(program, "MTWMatrix"), 1, GL_FALSE, glm::value_ptr(total));
	DrawModel(terrain, program, "in_Position", "in_Normal", "in_TexCoord");

	// Teapot:
	glm::vec3 teapotTrans = glm::vec3(0, 0, 0);
	trans = glm::translate(teapotTrans);
	scale = glm::scale(glm::vec3(0.5, 0.5, 0.5));
	total = trans * scale;
	glUniformMatrix4fv(glGetUniformLocation(program, "MTWMatrix"), 1, GL_FALSE, glm::value_ptr(total));
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
			loadNormConvModel();
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

void loadNormConvModel()
{
	bool isNODATA = true;
	while (isNODATA)
	{
		performNormConv();
		glReadPixels(0, 0, tW, tH, GL_RED, GL_FLOAT, dataHandler->getData());

		isNODATA = true;
		for (int i = 0; i < dataHandler->getElem() && isNODATA; i++)
		{
			float data = dataHandler->getData()[i];
			isNODATA = (data < 0.0001f);
		}
	}

	dataHandler->datamodel = dataHandler->GenerateTerrain(500.0f);
	terrain = dataHandler->datamodel;
}


void performNormConv()
{
	for (int i = 0; i < 5; i++)
	{
		// Filter original
		useFBO(fbo1, fbo3, 0L);

		// Clear framebuffer & zbuffer
		glClearColor(0.1, 0.1, 0.3, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Activate shader program
		glUseProgram(filtershader);
		glUniform2f(glGetUniformLocation(filtershader, "in_size"), tW, tH);

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		DrawModel(squareModel, filtershader, "in_Position", NULL, "in_TexCoord");


		// Create confidence
		useFBO(fbo2, fbo3, 0L);

		// Clear framebuffer & zbuffer
		glClearColor(0.1, 0.1, 0.3, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Activate shader program
		glUseProgram(confidenceshader);

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		DrawModel(squareModel, confidenceshader, "in_Position", NULL, "in_TexCoord");


		// Filter confidence
		useFBO(fbo3, fbo2, 0L);

		// Clear framebuffer & zbuffer
		glClearColor(0.1, 0.1, 0.3, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Activate shader program
		glUseProgram(filtershader);
		glUniform2f(glGetUniformLocation(filtershader, "in_size"), tW, tH);

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		DrawModel(squareModel, filtershader, "in_Position", NULL, "in_TexCoord");

		// Combine
		useFBO(fbo2, fbo1, fbo3);

		// Clear framebuffer & zbuffer
		glClearColor(0.1, 0.1, 0.3, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Activate shader program
		glUseProgram(combineshader);
		glUniform1i(glGetUniformLocation(combineshader, "dataTex"), 0);
		glUniform1i(glGetUniformLocation(combineshader, "confTex"), 1);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		DrawModel(squareModel, combineshader, "in_Position", NULL, "in_TexCoord");

		// Swap FBOs
		useFBO(fbo3, fbo2, 0L);

		// Clear framebuffer & zbuffer
		glClearColor(0.1, 0.1, 0.3, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Activate shader program
		glUseProgram(plaintextureshader);

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		DrawModel(squareModel, plaintextureshader, "in_Position", NULL, "in_TexCoord");
	}
}

// -----------------Ingemars hj�lpfunktioner-----------------
void reshape(int w, int h, glm::mat4 &projectionMatrix)
{
	glViewport(0, 0, w, h);
	float ratio = (GLfloat)w / (GLfloat)h;
	projectionMatrix = glm::perspective(PI / 2, ratio, 1.0f, 1000.0f);
}
// ----------------------------------------------------------
/*
Model* GenerateTerrain(TextureData *tex, GLfloat terrainScale)
{
	int vertexCount = tex->width * tex->height;
	int triangleCount = (tex->width - 1) * (tex->height - 1) * 2;
	int x, z;

	GLfloat *vertexArray = (GLfloat *)malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *normalArray = (GLfloat *)malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *texCoordArray = (GLfloat *)malloc(sizeof(GLfloat) * 2 * vertexCount);
	GLuint *indexArray = (GLuint *)malloc(sizeof(GLuint)* triangleCount * 3);

	glm::vec3 tempNormal = { 0, 0, 0 };

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

glm::vec3 giveNormal(int x, int y, int z, GLfloat *vertexArray, GLuint *indexArray, int width, int height)
{
	glm::vec3 vertex = { GLfloat(x), GLfloat(y), GLfloat(z) };
	glm::vec3 normal = { 0, 1, 0 };

	glm::vec3 normal1 = { 0, 0, 0 };
	glm::vec3 normal2 = { 0, 0, 0 };
	glm::vec3 normal3 = { 0, 0, 0 };
	glm::vec3 normal4 = { 0, 0, 0 };
	glm::vec3 normal5 = { 0, 0, 0 };
	glm::vec3 normal6 = { 0, 0, 0 };

	if ((x > 1) && (z > 1) && (z < height - 2) && (x < width - 2))
	{
		glm::vec3 tempVec1 = { vertexArray[indexArray[((x - 1) + (z - 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x - 1) + (z - 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x - 1) + (z - 1) * (width - 1)) * 6 + 0] * 3 + 2] };

		glm::vec3 tempVec2 = { vertexArray[indexArray[((x)+(z - 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x)+(z - 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x)+(z - 1) * (width - 1)) * 6 + 0] * 3 + 2] };

		glm::vec3 tempVec3 = { vertexArray[indexArray[((x - 1) + (z)* (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x - 1) + (z)* (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x - 1) + (z)* (width - 1)) * 6 + 0] * 3 + 2] };

		glm::vec3 tempVec4 = { vertexArray[indexArray[((x + 1) + (z)* (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x + 1) + (z)* (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x + 1) + (z)* (width - 1)) * 6 + 0] * 3 + 2] };

		glm::vec3 tempVec5 = { vertexArray[indexArray[((x + 1) + (z + 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x + 1) + (z + 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x + 1) + (z + 1) * (width - 1)) * 6 + 0] * 3 + 2] };

		glm::vec3 tempVec6 = { vertexArray[indexArray[((x)+(z + 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x)+(z + 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x)+(z + 1) * (width - 1)) * 6 + 0] * 3 + 2] };


		normal1 = glm::cross(tempVec1 - vertex, tempVec2 - vertex);
		normal2 = glm::cross(tempVec3 - vertex, tempVec1 - vertex);
		glm::vec3 weighted1 = normalize(normalize(normal1) + normalize(normal2));
		normal3 = glm::cross(tempVec2 - vertex, tempVec4 - vertex);
		glm::vec3 weighted2 = normalize(normal3);
		normal4 = glm::cross(tempVec4 - vertex, tempVec5 - vertex);
		normal5 = glm::cross(tempVec5 - vertex, tempVec6 - vertex);
		glm::vec3 weighted3 = normalize(normalize(normal4) + normalize(normal5));
		normal6 = glm::cross(tempVec6 - vertex, tempVec3 - vertex);
		glm::vec3 weighted4 = normalize(normal6);

		normal = normalize(weighted1 + weighted2 + weighted3 + weighted4);

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

		glm::vec3 p1 = { vertexArray[(vertX1 + vertZ1 * width) * 3 + 0], vertY1, vertexArray[(vertX1 + vertZ1 * width) * 3 + 2] };
		glm::vec3 p2 = { vertexArray[(vertX2 + vertZ2 * width) * 3 + 0], vertY2, vertexArray[(vertX2 + vertZ2 * width) * 3 + 2] };
		glm::vec3 p3 = { vertexArray[(vertX3 + vertZ3 * width) * 3 + 0], vertY3, vertexArray[(vertX3 + vertZ3 * width) * 3 + 2] };

		glm::vec3 planeNormal = { 0, 0, 0 };

		// This if/else might not be making any difference whatsoever.
		if (dist1 > dist2)
		{
			planeNormal = normalize(glm::cross(p2 - p1, p3 - p1));
		}
		else
		{
			planeNormal = normalize(glm::cross(p3 - p1, p2 - p1));
		}

		GLfloat D;
		D = glm::dot(planeNormal, p1);

		yheight = (D - planeNormal.x*x - planeNormal.z*z) / planeNormal.y;
	}
	return yheight;
}
*/
int main(int argc, char *argv[])
{
	init_SDL((const char*) "TSBB11, Waterflow visualization (SDL)", width, height);


	reshape(width, height, projectionMatrix);
	glEnableClientState(GL_VERTEX_ARRAY);
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
