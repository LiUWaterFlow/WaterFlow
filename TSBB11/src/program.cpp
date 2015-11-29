/// @file program.cpp
/// @brief Implementations of functions in program.h

#include "program.h"

#include "GL_utilities.h"
#include "voxelTesting.h"
#include "gtc/type_ptr.hpp"
#include <iostream>


bool updateRender;
bool sim = true;

Program::Program() {
	screenW = 800;
	screenH = 800;

	isRunning = true;
	mouseHidden = true;

	heightAtClickData = 0.0f;
	heighAtClickProj = 0.0f;

	// Time init
	currentTime = (GLfloat)SDL_GetTicks();
	deltaTime = 0;
}

Program::~Program() {}

int Program::exec() {
	if (!init()) return -1;

	SDL_Event Event;

	while (isRunning) {
		timeUpdate();
		while (SDL_PollEvent(&Event)) handleEvent(&Event);
		checkKeys();
		update();
		display();
	}

	clean();
	return 0;
}

int Program::testVoxels() {

	if (!init()) return -1;
	SDL_SetRelativeMouseMode(SDL_FALSE);
	voxelTest::mainTest(dataHandler);

}

bool Program::init() {
	// SDL, glew and OpenGL init
	updateRender = false;

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "Failed to initialise SDL: %s", SDL_GetError());
		return false;
	}

	#ifdef __APPLE__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	#endif


	screen = SDL_CreateWindow("TSBB11, Waterflow visualization (SDL)", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenW, screenH, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (screen == 0) {
		fprintf(stderr, "Failed to set Video Mode: %s", SDL_GetError());
		return false;
	}


	glcontext = SDL_GL_CreateContext(screen);


	SDL_SetRelativeMouseMode(SDL_TRUE);
	glClearColor(0.1f, 0.7f, 0.1f, 1.0f);
	printError("After SDL init: ");


	#ifdef _WINDOWS
	glewInit();
	#endif

	dumpInfo();


/* Functions below read start values and source-files form XML-file into a struct init_Data. Data from this
	 is then used as startdata.
*/
	const char* xmlfile = "src/xml/xgconsole.xml";

	init_Data_struct init_data(xmlfile);

	// Initial placement of camera.
	cam = new Camera(glm::vec3(0.0f,500.0f,0.0f), &screenW, &screenH);

	// Load terrain data
	dataHandler = new DataHandler(init_data.data_filename.c_str());
	
	// Initialize water simulation
	hf = new HeightField(dataHandler,init_data.FFData, init_data.Flowsources);
	hf->initGPU();
	
	printError("After hf init");

	// Load and compile shaders.
	terrainshader = loadShaders("src/shaders/terrainshader.vert", "src/shaders/terrainshader.frag");
	watershader = loadShaders("src/shaders/terrainshader.vert", "src/shaders/watershader.frag");
	skyshader = loadShaders("src/shaders/skyshader.vert", "src/shaders/skyshader.frag");

	// Create drawables
	GLuint sizes[] = { dataHandler->getDataWidth(), dataHandler->getDataHeight() };

	myDrawable::setLights();

	terrain = new HeightMap(terrainshader, sizes, dataHandler->getHeightBuffer());
	terrain->update();

	waterTerrain = new Water(watershader, sizes, hf->fieldBuffers[0], dynamic_cast<HeightMap*>(terrain)->getNormalBuffer());
	
	printError("Created Heightmaps");

	skycube = new SkyCube(skyshader);

	/*Initialize AntTweakBar
	*/
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(screenW, screenH);
	antBar = TwNewBar("UIinfo");
	TwDefine(" UIinfo refresh=0.1 ");
	TwDefine(" UIinfo valueswidth=fit ");
	TwDefine(" UIinfo size='280 250' ");
	TwAddVarRO(antBar, "FPS", TW_TYPE_FLOAT, &FPS, "group=Info");
	TwAddVarRO(antBar, "CameraX", TW_TYPE_FLOAT, &cam->getPos()->x, "group=Info");
	TwAddVarRO(antBar, "CameraZ", TW_TYPE_FLOAT, &cam->getPos()->z, "group=Info");

	TwAddVarRO(antBar, "Camera Height", TW_TYPE_FLOAT, &cam->getPos()->y, "group=Info");
	TwAddVarRO(antBar, "Terrain Height", TW_TYPE_FLOAT, &heightAtPos, "help= 'Shows terrain height at camera position' group=Info");

	TwAddVarRO(antBar, "Terrain Height (Proj)", TW_TYPE_FLOAT, &heighAtClickProj, "group=Click help= 'Shows terrain height at click using inverse projection' ");
	TwAddVarRO(antBar, "Terrain Height (Data)", TW_TYPE_FLOAT, &heightAtClickData, "group=Click help= 'Shows terrain height at click using terrain data for height' ");
	TwAddVarRO(antBar, "Terrain X", TW_TYPE_DOUBLE, &objX, "group=Click help= 'Shows terrain x at click' ");
	TwAddVarRO(antBar, "Terrain Z", TW_TYPE_DOUBLE, &objZ, "group=Click help= 'Shows terrain z at click' ");

	TwAddVarRW(antBar, "MovSpeed", TW_TYPE_FLOAT, cam->getSpeedPtr(), " min=0 max=10 step=0.05 group=Changables label='Movement speed' ");
	TwAddVarRW(antBar, "RotSpeed", TW_TYPE_FLOAT, cam->getRotSpeedPtr(), " min=0 max=0.1 step=0.0005 group=Changables label='Rotation speed' ");

	TwAddVarCB(antBar, "Transparency", TW_TYPE_FLOAT, Water::SetTransparencyCB, Water::GetTransparencyCB, waterTerrain, " min=0 max=1.0 step=0.05 group=Changables ");

	 TwAddVarCB(antBar, "Texture", TW_TYPE_INT32, HeightMap::SetTextureCB, HeightMap::GetTextureCB, terrain, " min=1 max=9 step=4 group=Changables ");

	printError("After total init: ");

	return true;
}

void Program::timeUpdate() {
	GLfloat t = (GLfloat)SDL_GetTicks();
	deltaTime = t - currentTime;
	currentTime = t;
	FPS = 1000.0f / deltaTime;
}

void Program::update() {
	// Update the tweak bar
	heightAtPos = dataHandler->giveHeight(cam->getPos()->x, cam->getPos()->z);
	if(sim){
		hf->runSimGPU();
	}
	waterTerrain->update();
}

void Program::display() {

	// Clear the screen.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// ====================== Draw skybox ===========================
	glUseProgram(skyshader);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// ---Camera shader data---
	cam->uploadCamData(skyshader);
	skycube->draw();

	// ====================== Draw Terrain ==========================
	glUseProgram(terrainshader);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// ---Camera shader data---
	cam->uploadCamData(terrainshader);
	terrain->draw();

	// ====================== Draw Terrain ==========================
	glUseProgram(watershader);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// ---Camera shader data---
	cam->uploadCamData(watershader);
	glUniform1f(glGetUniformLocation(watershader, "time"), currentTime / 100.0f);
	waterTerrain->draw();
	
	// ====================== Draw AntBar ===========================
	TwDraw();

	printError("after display");

	SDL_GL_SwapWindow(screen);
}

void Program::clean() {
	TwTerminate();
	SDL_GL_DeleteContext(glcontext);
	SDL_Quit();
}


// Handle events.
void Program::handleEvent(SDL_Event* event) {
	switch (event->type) {
		case SDL_QUIT:
		isRunning = false;
		break;
		case SDL_KEYDOWN:
		handleKeypress(event);
		break;
		case SDL_WINDOWEVENT:
		switch (event->window.event) {
			case SDL_WINDOWEVENT_RESIZED:
			SDL_SetWindowSize(screen, event->window.data1, event->window.data2);
			SDL_GetWindowSize(screen, &screenW, &screenH);
			glViewport(0, 0, screenW, screenH);
			TwWindowSize(screenW, screenH);
			cam->updateVTP();
			break;
		}
		break;
		case SDL_MOUSEMOTION:
		handleMouseMove(event);
		break;
		case SDL_MOUSEBUTTONDOWN:
		TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_LEFT);
		handleMouseButton(event);
		break;
		case SDL_MOUSEBUTTONUP:
		TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_LEFT);
		break;
		default:
		break;
	}
}

// Handle keys
void Program::handleKeypress(SDL_Event* event) {
	TwKeyPressed(event->key.keysym.sym, TW_KMOD_NONE);
	switch (event->key.keysym.sym) {
		case SDLK_ESCAPE:
		isRunning = false;
		break;
		case SDLK_h:
		cam->toggleFrozen();
		mouseHidden = !mouseHidden;
		if (!mouseHidden) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
		} else {
			SDL_SetRelativeMouseMode(SDL_TRUE);
		}
		break;
		case SDLK_r:
		int isBarHidden;
		TwGetParam(antBar, NULL, "iconified", TW_PARAM_INT32, 1, &isBarHidden);
		if (!isBarHidden) {
			TwDefine(" UIinfo iconified=true");
		} else {
			TwDefine(" UIinfo iconified=false");
		}
		case SDLK_l:
		sim = false;
		hf->measureVolume();
		break;
		case SDLK_k:
		sim = !sim;
		break;
		default:
		break;
	}
}

void Program::handleMouseMove(SDL_Event* event) {
	if (!mouseHidden) {
		TwMouseMotion(event->motion.x, event->motion.y);
	}
	cam->changeLookAtPos(event->motion.xrel, event->motion.yrel);
}

void Program::handleMouseButton(SDL_Event* event) {
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	if (keystate[SDL_SCANCODE_LSHIFT] && !mouseHidden) {
		float depth;
		int x;
		int y;
		GLint viewport[4] = { 0, 0, 0, 0 };
		glGetIntegerv(GL_VIEWPORT, viewport);
		SDL_GetMouseState(&x, &y);
		glReadPixels(x, viewport[3] - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
		GLdouble objY = 0.0;
		gluUnProject((GLdouble)x, (GLdouble)(viewport[3] - y), (GLdouble)depth, glm::value_ptr((glm::dmat4)*cam->getWTV()), glm::value_ptr((glm::dmat4)*cam->getVTP()), viewport, &objX, &objY, &objZ);

		heighAtClickProj = (GLfloat)objY;
		heightAtClickData = dataHandler->giveHeight((GLfloat)objX, (GLfloat)objZ);
	}
}

void Program::checkKeys() {
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	if (keystate[SDL_SCANCODE_W]) {
		cam->forward(deltaTime);
	} else if (keystate[SDL_SCANCODE_S]) {
		cam->forward(-deltaTime);
	}
	if (keystate[SDL_SCANCODE_A]) {
		cam->strafe(deltaTime);
	} else if (keystate[SDL_SCANCODE_D]) {
		cam->strafe(-deltaTime);
	}
	if (keystate[SDL_SCANCODE_Q]) {
		cam->jump(deltaTime);
	} else if (keystate[SDL_SCANCODE_E]) {
		cam->jump(-deltaTime);
	} 
	if (keystate[SDL_SCANCODE_T]) {
		hf->initTest();
	} else if (keystate[SDL_SCANCODE_U]) {
		updateRender = !updateRender;
	}
}
