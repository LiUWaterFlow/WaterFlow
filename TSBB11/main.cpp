// The code below is based upon Ingemar Ragnemalm's code provided for the
// course TSBK03 at Linköping University. The link to the original shell is
// http://www.ragnemalm.se/lightweight/psychteapot+MicroGlut-Windows-b1.zip
// which according to the web site was updated 2015-08-17.


#ifdef __APPLE__
// Mac
	#include <OpenGL/gl3.h>
	// uses framework Cocoa
#else
	#ifdef _WIN32
// MS
		#include <windows.h>
		#include <stdio.h>
		#include "glew.h"
	#else
// Linux
		#include <stdio.h>
		#include <GL/gl.h>
//		#include <GL/glut.h>
	#endif
#endif

#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"
#include "zpr.h"
#include <cstdlib>

#include "readData.h"

#include <iostream>

#ifndef NULL
#define NULL 0L
#endif
#ifndef PI
#define PI 3.14159265358979323846f
#endif

//==============================

// initial width and heights

GLfloat square[] = {
							-1,-1,0,
							-1,1, 0,
							1,1, 0,
							1,-1, 0};
GLfloat squareTexCoord[] = {
							 0, 0,
							 0, 1,
							 1, 1,
							 1, 0};
GLuint squareIndices[] = {0, 1, 2, 0, 2, 3};

Model* squareModel;
FBOstruct *fbo1, *fbo2, *fbo3;
GLuint plaintextureshader = 0, filtershader = 0, confidenceshader = 0, combineshader = 0;
Point3D cam, point;

void OnTimer(int value)
{
	glutPostRedisplay();
	glutTimerFunc(5, &OnTimer, value);
}

void resize(int w, int h) // TEST
{
	glViewport(0,0,w, h);
	glutPostRedisplay();
}

mat4 viewMatrix;

int W, H;
//==============================

mat4 projectionMatrix, camMatrix;

Model *m;
Model *terrain;
TextureData ttex; // Dummy terrain.
// Reference to shader program
GLuint program;

//vec3 cam(10, 10, 10);
vec3 lookAtPoint(2, 0, 2);
vec3 v(0.0, 1.0, 0.0);
vec3 s = lookAtPoint - cam;

// Some basic functions. TODO: Move appropriate ones (most of them) to separate source file(s).
Model* GenerateTerrain(TextureData *tex, GLfloat terrainScale); // Generates a model given a height map (grayscale .tga file for now).
vec3 giveNormal(int x, int y, int z, GLfloat *vertexArray, GLuint *indexArray, int width, int height); // Returns the normal of a vertex.
GLfloat giveHeight(GLfloat x, GLfloat z, GLfloat *vertexArray, int width, int height); // Returns the height of a height map.

FBOstruct *initFBO3(int width, int height, void* data)
{
	FBOstruct *fbo = (FBOstruct*)malloc(sizeof(FBOstruct));

	fbo->width = width;
	fbo->height = height;

	// create objects
	glGenFramebuffers(1, &fbo->fb); // frame buffer id
	glBindFramebuffer(GL_FRAMEBUFFER, fbo->fb);
	glGenTextures(1, &fbo->texid);
	fprintf(stderr, "%i \n",fbo->texid);
	glBindTexture(GL_TEXTURE_2D, fbo->texid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->texid, 0);

	// Renderbuffer
	// initialize depth renderbuffer
    glGenRenderbuffers(1, &fbo->rb);
    glBindRenderbuffer(GL_RENDERBUFFER, fbo->rb);
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, fbo->width, fbo->height );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo->rb );
    CHECK_FRAMEBUFFER_STATUS();

	fprintf(stderr, "Framebuffer object %d\n", fbo->fb);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return fbo;
}


void init(void)
{
	dumpInfo();  // shader info
	
	// GL inits
	glClearColor(0.1, 0.1, 0.3, 0);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	printError("GL inits");

	// Load and compile shader
	program = loadShaders("shaders/main.vert", "shaders/main.frag");
	plaintextureshader = loadShaders("resources/plaintextureshader.vert", "resources/plaintextureshader.frag");  // puts texture on teapot
	filtershader = loadShaders("resources/plaintextureshader.vert", "resources/filtershader.frag"); 
	confidenceshader = loadShaders("resources/plaintextureshader.vert", "resources/confidenceshader.frag"); 
	combineshader = loadShaders("resources/plaintextureshader.vert", "resources/combineshader.frag"); 
	glUseProgram(program);
	
	// Upload geometry to the GPU:
	DataHandler test("resources/output.min.asc",1.0f);
	//LoadTGATextureData("resources/fft-terrain.tga", &ttex);
	//terrain = test.datamodel;
	m = LoadModelPlus("resources/teapot.obj");
	
	W = test.getWidth();
	H = test.getHeight();
	
	resize(W, H);
	
	fbo1 = initFBO3(W, H, NULL);
	fbo2 = initFBO3(W, H, NULL);
	fbo3 = initFBO3(W, H, test.getData());

	
	printError("init shader");

	squareModel = LoadDataToModel(square, NULL, squareTexCoord, NULL, squareIndices, 4, 6);

	
	
	// End of upload of geometry
	/*
	projectionMatrix = frustum(-0.5, 0.5, -0.5, 0.5, 1.0, 30.0);
	camMatrix = lookAt(0, 1, 8, 0,0,0, 0,1,0);
	
	glUniformMatrix4fv(glGetUniformLocation(program, "camMatrix"), 1, GL_TRUE, camMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	*/

	cam = SetVector(0, 5, 15);
	point = SetVector(0, 1, 0);

	//glutTimerFunc(50, &OnTimer, 0);

	zprInit(&viewMatrix, cam, point);
	
}

GLfloat a, b = 0.0;

void display(void)
{
	for(int i = 0; i < 1; i++)
	{
		// Filter original
		useFBO(fbo1, fbo3, 0L);

		// Clear framebuffer & zbuffer
		glClearColor(0.1, 0.1, 0.3, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Activate shader program
		glUseProgram(filtershader);
		glUniform2f(glGetUniformLocation(filtershader, "in_size"), W, H);

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
		glUniform2f(glGetUniformLocation(filtershader, "in_size"), W, H);

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
		
		// render to fbo1!
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

	// render to fbo1!
	useFBO(0L, fbo3, 0L);

	// Clear framebuffer & zbuffer
	glClearColor(0.1, 0.1, 0.3, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate shader program
	glUseProgram(plaintextureshader);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	DrawModel(squareModel, plaintextureshader, "in_Position", NULL, "in_TexCoord");

	

	glutSwapBuffers();
}

void mouse(int x, int y)
{
	b = x * 1.0f;
	a = y * 1.0f;
	glutPostRedisplay();
}


/*void idle()
{
  glutPostRedisplay();
}*/
int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutInitWindowSize(260, 540);
	glutCreateWindow ("Ingemar's psychedelic teapot 2");

#ifdef WIN32
	glewInit();
#endif
	glutDisplayFunc(display); 
	glutPassiveMotionFunc(mouse);
	glutRepeatingTimer(200);
	glutReshapeFunc(resize);
	//glutIdleFunc(idle);
	init ();
	
	glutMainLoop();
	exit(0);
}

