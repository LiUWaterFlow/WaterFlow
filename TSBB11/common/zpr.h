#ifndef ZPR_H
#define ZPR_H

/*
 * Zoom-pan-rotate mouse manipulation module for GLUT
 *
 * Originally written by Nigel Stewart
 *
 * Hacked quite a bit by Mikael Kalms
 *
 * Hacked quite a bit more by Ingemar Ragnemalm 2013, for using
 * VectorUtils3.
 */

#ifdef __APPLE__
// Mac
	#include <OpenGL/gl3.h>
	#include "MicroGlut.h"
#else
	#ifdef _WIN32
// MS
		#include <windows.h>
		#include <stdio.h>
		#include "glew.h"
		#include "freeglut.h"
	#else
// Linux
		#include <stdio.h>
		#include <GL/gl.h>
		#include "MicroGlut.h"
	#endif
#endif

#include "VectorUtils3.h"

#ifdef __cplusplus
extern "C"
{
#endif

void zprInit(mat4 *viewMatrix, vec3 cam, vec3 point);

void updateCameraMatrix(mat4 *matrix);    // matrix is passed as a parameter when both cameraMatrix
                                                 // and camera and lookAtPoint have to be transformed.
                                                 //  if matrix = NULL, only cameraMatrix is updated

void zprMouse(int button, int state, int x, int y);  // records mouse buttons' state
void zprKey(unsigned char key, int x, int y); // camera control with keys. Updates viewMatrix
void zprMouseFunc(int x, int y);

#ifdef __cplusplus
}
#endif

#endif
