/// @file Utilities.c
/// This file implements useful functions not part of the other libraries.

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

#include "Utilities.h"
#include "loadobj.h"
#include <stdlib.h>

Model* generateCanvas()
{
	// Create canvas to draw on
	GLfloat square[] = { -1, -1, 0,
		-1, 1, 0,
		1, 1, 0,
		1, -1, 0 };
	GLfloat squareTexCoord[] = { 0, 0,
		0, 1,
		1, 1,
		1, 0 };
	GLuint squareIndices[] = { 0, 1, 2, 0, 2, 3 };

	return LoadDataToModel(square, NULL, squareTexCoord, NULL, squareIndices, 4, 6);
	free(squareIndices);
	free(square);
	free(squareTexCoord);
}

Model* generateCube(GLfloat s) {
	GLfloat vertexArray[3 * 8] = {	-s, -s, s,
									s, -s, s,
									s, s, s,
									-s, s, s,
									-s, -s, -s,
									s, -s, -s,
									s, s, -s,
									-s, s, -s};

	GLuint indexArray[6 * 2 * 3] = {0, 1, 2, 2, 3, 0, 
									3, 2, 6, 6, 7, 3, 
									7, 6, 5, 5, 4, 7, 
									4, 0, 3, 3, 7, 4, 
									0, 1, 5, 5, 4, 0, 
									1, 5, 6, 6, 2, 1};

	// Create Model and upload to GPU.
	return LoadDataToModel(vertexArray,	NULL, NULL, NULL, indexArray, 8, 6 * 2 * 3);
}

//Only to be used for 'final destruction' of models. (they will not be renderable after this.)
void releaseModel(Model* m)
{
	if (m != NULL)
	{
		glDeleteBuffers(1,&m->vb);
		glDeleteBuffers(1,&m->nb);
		glDeleteBuffers(1,&m->tb);
		glDeleteBuffers(1,&m->ib);
		glDeleteVertexArrays(1,&m->vao);
		free(m);
	}
}
