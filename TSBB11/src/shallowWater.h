#ifndef SHALLOWWATER_H
#define SHALLOWWATER_H

#ifdef __APPLE__
#	include <OpenGL/gl3.h>
#	include <SDL2/SDL.h>
#else
#	ifdef  __linux__
#		define GL_GLEXT_PROTOTYPES
#		include <GL/gl.h>
#		include <GL/glu.h>
#		include <GL/glx.h>
#		include <GL/glext.h>
#		include <SDL2/SDL.h>
#	else
#		include "glew.h"
#		include "Windows/sdl2/SDL.h"
#	endif
#endif

class shallowWater {
private:
	GLuint N;
	GLfloat g, dt, dx, dy;

	GLfloat **H, **Hx, **Hy;
	GLfloat **U, **Ux, **Uy;
	GLfloat **V, **Vx, **Vy;

	void set_bnd(int N, int b, float** x);
	void firsthalfstep(float** H, float** U, float** V, float** Hx, float** Ux, float** Vx, float** Hy, float** Uy, float** Vy, float dt, float dx, float dy, float g, int N);
	void secondhalfstep(float** H, float** U, float** V, float** Hx, float** Ux, float** Vx, float** Hy, float** Uy, float** Vy, float dt, float dx, float dy, float g, int N);

public:
	shallowWater(GLuint gridSize);

	~shallowWater();

	void init();
	void run();
	void printH();

	GLfloat** getHeight() { return H; }

};

#endif // SHALLOWWATER_H