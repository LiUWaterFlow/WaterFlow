/// @file Utilities.c
/// This file implements useful functions not part of the other libraries.


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
}

void releaseModel(Model* m)
{
	if (m != NULL)
	{
		free(m->vertexArray);
		free(m->normalArray);
		free(m->texCoordArray);
		free(m->colorArray);
		free(m->indexArray);
	}
	free(m);
}