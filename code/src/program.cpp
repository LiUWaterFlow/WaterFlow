/// @file program.cpp
/// @brief Implementations of functions in program.h

#include "program.h"

#include "GL_utilities.h"
#include "voxelTesting.h"
#include "gtc/type_ptr.hpp"
#include <iostream>


Program::Program(GLuint simCaseIn) {
	simCase = simCaseIn;
	screenW = 800;
	screenH = 800;
	
	sim = true;

	isRunning = true;
	mouseHidden = true;

	heightAtClickData = 0.0f;
	heighAtClickProj = 0.0f;

	// Time init
	currentTime = (GLfloat)SDL_GetTicks();
	deltaTime = 0;
	dtSim = 1.0f / (2.0f*20.0f*30.0f);
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
	
	return 0;

}
bool Program::init() {
	// SDL, glew and OpenGL init
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "Failed to initialise SDL: %s", SDL_GetError());
		return false;
	}

#ifdef __APPLE__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif


	screen = SDL_CreateWindow("WaterFlow", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenW, screenH, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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
	//Check for compatibility.
	int numBindings;
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &numBindings);

	if (simCase == 1) {
		if (numBindings > 8) {

		}
		else {
			std::cerr << "This graphics card has too few Shader Storage Buffer bindings" << std::endl;
			system("pause");
			return false;
		}
	}if (simCase == 2) {
		if (numBindings > 12) {

		}
		else {
			std::cerr << "This graphics card has too few Shader Storage Buffer bindings" << std::endl;
			system("pause");
			return false;
		}
	}




	dumpInfo();
	/* Functions below read start values and source-files form XML-file into a struct init_data-> Data from this
	is then used as startdata.
	*/
	const char* xmlfile = "src/xml/xgconsole.xml";

	init_data = new init_Data_struct(xmlfile);
	// Load terrain data.
	dataHandler = new DataHandler(init_data->data_filename.c_str());

	// Load previous simulation
	heightData = new FileHandler(dataHandler->getDataWidth(), dataHandler->getDataHeight());
	velocityData = new FileHandler(dataHandler->getDataWidth(), dataHandler->getDataHeight());

	heightData->LoadData(init_data->height_load_path.c_str());
	velocityData->LoadData(init_data->velocity_load_path.c_str());

	// Initial placement of camera.
	int xzLim = 250; // Maximal distance (x and z) between terrain and camera
	int yLimLo = 100; // Minimal distance (y) between camera and terrain
	int yLimHi = 500; // yLimHi + dataHandler->getTerrainScale() = maximal distance (y) between camera and terrain
	cam = new Camera(glm::vec3(0.0f, 500.0f, 0.0f), &screenW, &screenH, dataHandler->getDataWidth(), dataHandler->getDataHeight(), xzLim, yLimLo, yLimHi, dataHandler);
	
	printError("After hf init");
	
	// Load and compile shaders.
	terrainshader = loadShaders("src/shaders/terrainshader.vert", "src/shaders/terrainshader.frag");
	skyshader = loadShaders("src/shaders/skyshader.vert", "src/shaders/skyshader.frag");
	depthshader = loadShaders("src/shaders/terrainshader.vert", "src/shaders/depthshader.frag");
	
	GLuint sizes[] = {(GLuint)dataHandler->getDataWidth(),(GLuint)dataHandler->getDataHeight() };		
	    
	// Create drawables
	myDrawable::setLights();
	myDrawable::setTextures(sizes);

	if (simCase == 1) {
		terrain = new HeightMap(terrainshader, sizes, dataHandler->getTerrainScale(), dataHandler->getHeightBuffer());
		terrain->update();
		dynamic_cast<HeightMap*>(terrain)->generateHeightTexture();
		
		printError("Created Terrain");
	
		watershader = loadShaders("src/shaders/terrainshader.vert", "src/shaders/watershader.frag");

		// Initialize water simulation
		hf = new HeightField(dataHandler, init_data->FFData, init_data->Flowsources);
		hf->initGPU(heightData->GetArray(), velocityData->GetArray());

		GLuint shaders[2] = { watershader, depthshader };
		waterTerrain = new Water(shaders, sizes, dataHandler->getTerrainScale(), hf->fieldBuffers[0]);
	} else if(simCase == 2)	{
		terrain = new HeightMap(terrainshader, sizes, -1.0f, dataHandler->getHeightBuffer());
		terrain->update();
		dynamic_cast<HeightMap*>(terrain)->generateHeightTexture();
		
		printError("Created Terrain");
	
		watershader = loadShaders("src/shaders/terrainshader.vert", "src/shaders/shallowwatershader.frag");
		cam->unlock();

		// Initialize water simulation
		sgpu = new ShallowGPU(dataHandler, init_data->FFData);
		sgpu->initGPU();

		GLuint shaders[2] = { watershader, depthshader };
		waterTerrain = new Water(shaders, sizes, dataHandler->getTerrainScale(), sgpu->shallowBuffers[7]);
	}
		
	printError("Created Water");
	skycube = new SkyCube(skyshader);


	/* Initialize AntTweakBar */
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(screenW, screenH);
	antBar = TwNewBar("UIinfo");
	TwDefine(" UIinfo refresh=0.1 ");
	TwDefine(" UIinfo valueswidth=fit ");
	TwDefine(" UIinfo size='280 350' ");
	TwAddVarRO(antBar, "FPS", TW_TYPE_FLOAT, &FPS, "group=Info");
	TwAddVarRO(antBar, "CameraX", TW_TYPE_FLOAT, &cam->getPos()->x, "group=Info");
	TwAddVarRO(antBar, "CameraZ", TW_TYPE_FLOAT, &cam->getPos()->z, "group=Info");

	TwAddVarRO(antBar, "Camera Height", TW_TYPE_FLOAT, &cam->getPos()->y, "group=Info");
	TwAddVarRO(antBar, "TerrainHeightCamera", TW_TYPE_FLOAT, &heightAtPos, " label='Terrain Height' help= 'Shows terrain height at camera position' group=Info");

	TwAddVarRO(antBar, "Water Depth", TW_TYPE_FLOAT, &heighAtClickProj, "group=Click help= 'Shows water depth at click using inverse projection' ");
	TwAddVarRO(antBar, "Terrain Height", TW_TYPE_FLOAT, &heightAtClickData, "group=Click help= 'Shows terrain height at click using terrain data for height' ");
	TwAddVarRO(antBar, "Terrain X", TW_TYPE_DOUBLE, &objX, "group=Click help= 'Shows terrain x at click' ");
	TwAddVarRO(antBar, "Terrain Z", TW_TYPE_DOUBLE, &objZ, "group=Click help= 'Shows terrain z at click' ");

	TwAddVarRW(antBar, "MovSpeed", TW_TYPE_FLOAT, cam->getSpeedPtr(), " min=0 max=10 step=0.05 group=Changables label='Movement speed' ");
	TwAddVarRW(antBar, "RotSpeed", TW_TYPE_FLOAT, cam->getRotSpeedPtr(), " min=0 max=0.1 step=0.0005 group=Changables label='Rotation speed' ");

	TwAddVarCB(antBar, "Terrain Texture", TW_TYPE_INT32, HeightMap::SetTextureCB, HeightMap::GetTextureCB, terrain, " min=0 max=2 step=1 group=Changables ");
	TwAddVarCB(antBar, "Bottom Texture", TW_TYPE_INT32, HeightMap::SetTextureCB, HeightMap::GetTextureCB, waterTerrain, " min=0 max=2 step=1 group=Changables ");

	TwAddVarCB(antBar, "Draw Program", TW_TYPE_INT32, Water::SetDrawProgramCB, Water::GetDrawProgramCB, waterTerrain, " min=0 max=1 step=1 group=Changables ");
	TwAddVarCB(antBar, "Transparency", TW_TYPE_FLOAT, Water::SetTransparencyCB, Water::GetTransparencyCB, waterTerrain, " min=0 max=1.0 step=0.05 group=Changables ");
	TwAddVarCB(antBar, "MaxDepth", TW_TYPE_FLOAT, Water::SetMaxDepthCB, Water::GetMaxDepthCB, waterTerrain, " min=0 max=500.0 step=10.0 group=Changables ");
	TwAddVarRW(antBar, "Simulation dt", TW_TYPE_FLOAT, &dtSim, "min=0.00005 max=0.0025 step=0.00005 group=Changables label='Simulation dt' ");
	
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
	// Update the tweak bar.
	heightAtPos = dataHandler->giveHeight(cam->getPos()->x, cam->getPos()->z);
		
	
	if (simCase == 1)
	{
		
		if (sim) {
			hf->runSimGPU(dtSim);
		}
	
	
	
	}else if(simCase == 2)
	{
		if (sim) {
			sgpu->runSimGPU();
		}
	}
	
	
	waterTerrain->update();

}


void Program::display() {
	// Clear the screen.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// ========================== Draw skybox ==========================
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	// ---Camera shader data---
	cam->uploadCamData(skyshader);
	skycube->draw();

	// ========================== Draw Terrain =========================
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// ---Camera shader data---
	cam->uploadCamData(terrainshader);
	if(simCase != 2){
		terrain->draw();
	}

	// ======================== Draw water body ========================
	glDisable(GL_CULL_FACE);

	cam->uploadCamData(depthshader);
	cam->uploadCamData(watershader);
	glUniform1f(glGetUniformLocation(watershader, "time"), currentTime / 20.0f);
	waterTerrain->draw();

	// ========================== Draw AntBar ==========================
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

// Handle keys.
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
		break;
		
	case SDLK_k:
		if(simCase == 1){
			hf->saveData(heightData->GetArray(),velocityData->GetArray());
			heightData->SaveData(init_data->height_save_path.c_str()); 
			velocityData->SaveData(init_data->velocity_save_path.c_str()); 
			
		}
		break;
	case SDLK_p:
		sim = !sim;
		break;
	case SDLK_c:
		if (simCase != 2)
			break;
		sgpu->cycleBuffer();
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

		heightAtClickData = dataHandler->giveHeight((GLfloat)objX, (GLfloat)objZ);
		heighAtClickProj = fmax((GLfloat)objY - heightAtClickData, 0.0f);
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
}
