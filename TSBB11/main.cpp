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
#include <cstdlib>

#include "readData.h"

#include <iostream>

#ifndef NULL
#define NULL 0L
#endif
#ifndef PI
#define PI 3.14159265358979323846
#endif

mat4 projectionMatrix, camMatrix;

Model *m;
Model *terrain;
TextureData ttex; // Dummy terrain.
// Reference to shader program
GLuint program;

vec3 cam(10, 10, 10);
vec3 lookAtPoint(2, 0, 2);
vec3 v(0.0, 1.0, 0.0);
vec3 s = lookAtPoint - cam;

// Some basic functions. TODO: Move appropriate ones (most of them) to separate source file(s).
Model* GenerateTerrain(TextureData *tex, GLfloat terrainScale); // Generates a model given a height map (grayscale .tga file for now).
vec3 giveNormal(int x, int y, int z, GLfloat *vertexArray, GLuint *indexArray, int width, int height); // Returns the normal of a vertex.
GLfloat giveHeight(GLfloat x, GLfloat z, GLfloat *vertexArray, int width, int height); // Returns the height of a height map.

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
	LoadTGATextureData("resources/fft-terrain.tga", &ttex);
	m = LoadModelPlus("resources/teapot.obj");
	terrain = GenerateTerrain(&ttex, 2);
	// End of upload of geometry
	
	projectionMatrix = frustum(-0.5, 0.5, -0.5, 0.5, 1.0, 30.0);
	camMatrix = lookAt(0, 1, 8, 0,0,0, 0,1,0);
	
	glUniformMatrix4fv(glGetUniformLocation(program, "camMatrix"), 1, GL_TRUE, camMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
}

GLfloat a, b = 0.0;

void display(void)
{
	mat4 rot, trans, scale, total, tMat;
	GLfloat t;

	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	t = glutGet(GLUT_ELAPSED_TIME) / 100.0;

	trans = T(-5, -10, 20); // Move teapot to center it
	scale = S(10, 10, 10);
	rot = Mult(Ry(b / 50), Rx(a / 50)); // Rotation by mouse movements
	total = Mult(Mult(rot, trans), Rx(-M_PI / 2)); // Rx rotates the teapot to a comfortable default
	glUniformMatrix4fv(glGetUniformLocation(program, "mdlMatrix"), 1, GL_TRUE, total.m);
	glUniform1fv(glGetUniformLocation(program, "t"), 1, &t); // Use glUniform1fv because glUniform1f has a bug under Linux!
	DrawModel(terrain, program, "inPosition", NULL, "inTexCoord");

	trans = T(0, -1, 0); // Move teapot to center it
	rot = Mult(Ry(b/50), Rx(a/50)); // Rotation by mouse movements
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

Model* GenerateTerrain(TextureData *tex, GLfloat terrainScale) // Generates a model given a height map (grayscale .tga file for now).
{
	int vertexCount = tex->width * tex->height;
	int triangleCount = (tex->width - 1) * (tex->height - 1) * 2;
	int x, z;

	GLfloat *vertexArray = (GLfloat *)malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *normalArray = (GLfloat *)malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *texCoordArray = (GLfloat *)malloc(sizeof(GLfloat) * 2 * vertexCount);
	GLuint *indexArray = (GLuint *)malloc(sizeof(GLuint)* triangleCount * 3);

	vec3 tempNormal( 0, 0, 0 );

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

vec3 giveNormal(int x, int y, int z, GLfloat *vertexArray, GLuint *indexArray, int width, int height) // Returns the normal of a vertex.
{
	vec3 vertex( x, y, z );
	vec3 normal( 0, 1, 0 );

	vec3 normal1( 0, 0, 0 );
	vec3 normal2( 0, 0, 0 );
	vec3 normal3( 0, 0, 0 );
	vec3 normal4( 0, 0, 0 );
	vec3 normal5( 0, 0, 0 );
	vec3 normal6( 0, 0, 0 );

	if ((x > 1) && (z > 1) && (z < height - 2) && (x < width - 2))
	{
		vec3 tempVec1( vertexArray[indexArray[((x - 1) + (z - 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x - 1) + (z - 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x - 1) + (z - 1) * (width - 1)) * 6 + 0] * 3 + 2] );

		vec3 tempVec2( vertexArray[indexArray[((x)+(z - 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x)+(z - 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x)+(z - 1) * (width - 1)) * 6 + 0] * 3 + 2] );

		vec3 tempVec3( vertexArray[indexArray[((x - 1) + (z)* (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x - 1) + (z)* (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x - 1) + (z)* (width - 1)) * 6 + 0] * 3 + 2] );

		vec3 tempVec4( vertexArray[indexArray[((x + 1) + (z)* (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x + 1) + (z)* (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x + 1) + (z)* (width - 1)) * 6 + 0] * 3 + 2] );

		vec3 tempVec5( vertexArray[indexArray[((x + 1) + (z + 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x + 1) + (z + 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x + 1) + (z + 1) * (width - 1)) * 6 + 0] * 3 + 2] );

		vec3 tempVec6( vertexArray[indexArray[((x)+(z + 1) * (width - 1)) * 6 + 0] * 3],
			vertexArray[indexArray[((x)+(z + 1) * (width - 1)) * 6 + 0] * 3 + 1],
			vertexArray[indexArray[((x)+(z + 1) * (width - 1)) * 6 + 0] * 3 + 2] );


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

GLfloat giveHeight(GLfloat x, GLfloat z, GLfloat *vertexArray, int width, int height) // Returns the height of a height map.
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

		vec3 p1( vertexArray[(vertX1 + vertZ1 * width) * 3 + 0], vertY1, vertexArray[(vertX1 + vertZ1 * width) * 3 + 2] );
		vec3 p2( vertexArray[(vertX2 + vertZ2 * width) * 3 + 0], vertY2, vertexArray[(vertX2 + vertZ2 * width) * 3 + 2] );
		vec3 p3( vertexArray[(vertX3 + vertZ3 * width) * 3 + 0], vertY3, vertexArray[(vertX3 + vertZ3 * width) * 3 + 2] );

		vec3 planeNormal( 0, 0, 0 );

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