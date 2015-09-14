// The code below is based upon Ingemar Ragnemalm's code provided for the
// course TSBK03 at Linköping University. The link to the original shell is
// http://www.ragnemalm.se/lightweight/psychteapot+MicroGlut-Windows-b1.zip
// which according to the web site was updated 2015-08-17.


#ifdef __APPLE__// Mac	#include <OpenGL/gl3.h>	// uses framework Cocoa#else	#ifdef _WIN32// MS		#include <windows.h>		#include <stdio.h>		#include "glew.h"	#else// Linux		#include <stdio.h>		#include <GL/gl.h>//		#include <GL/glut.h>	#endif#endif

#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"
#include <cstdlib>

#include "readData.h"

#include <iostream>

#ifndef NULL
#define NULL 0L
#endif

mat4 projectionMatrix, camMatrix;

Model *m;
// Reference to shader program
GLuint program;

void init(void)
{
	// GL inits
	glClearColor(0.3,0.3,0.3,0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_TRUE);

	// Load and compile shader
	program = loadShaders("shaders/main.vert", "shaders/main.frag");
	glUseProgram(program);
	
	// Upload geometry to the GPU:
	m = LoadModelPlus("resources/teapot.obj");
	// End of upload of geometry
	
	projectionMatrix = frustum(-0.5, 0.5, -0.5, 0.5, 1.0, 30.0);
	camMatrix = lookAt(0, 1, 8, 0,0,0, 0,1,0);
	
	glUniformMatrix4fv(glGetUniformLocation(program, "camMatrix"), 1, GL_TRUE, camMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
}

GLfloat a, b = 0.0;

void display(void)
{
	mat4 rot, trans, total, tMat;
	GLfloat t;

	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	t = glutGet(GLUT_ELAPSED_TIME) / 100.0;

	trans = T(0, -1, 0); // Move teapot to center it
	rot = Mult(Ry(b/20), Rx(a/20)); // Rotation by mouse movements
	total = Mult(Mult(rot, trans), Rx(-M_PI/2)); // Rx rotates the teapot to a comfortable default
	glUniformMatrix4fv(glGetUniformLocation(program, "mdlMatrix"), 1, GL_TRUE, total.m);
	glUniform1fv(glGetUniformLocation(program, "t"), 1, &t); // Use glUniform1fv because glUniform1f has a bug under Linux!

	DrawModel(m, program, "inPosition", NULL, "inTexCoord");
	
	glutSwapBuffers();
}

void mouse(int x, int y)
{
	b = x * 1.0;
	a = y * 1.0;
	glutPostRedisplay();
}

void resize(int w, int h) // TEST
{
	glViewport(0,0,w, h);
	glutPostRedisplay();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutCreateWindow ("Ingemar's psychedelic teapot 2");
#ifdef WIN32
	glewInit();
#endif
	glutDisplayFunc(display); 
	glutPassiveMotionFunc(mouse);
	glutRepeatingTimer(20);
	glutReshapeFunc(resize);
	init ();
	
	//mapdata* test = readDEM("../../Data/output.min.asc");
	
	glutMainLoop();
	exit(0);
}
