/// @file camera.h
/// @brief Pretty general camera class
///
/// Written by Gustav Svensk, free to use as long as this note about the original author remain

#ifndef CAMERA_H
#define CAMERA_H

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



#include "glm.hpp"
#include "readData.h"

/// @class Camera
/// @brief Pretty general camera class
class Camera {
private:
	// x används för att musen inte ska fastna i kanterna på
	// fönstret
	int x;
	float fi, theta, rotSpeed;
	int *screenW, *screenH;
	GLfloat speed, drawDistance;
	bool isFrozen;
	int terrW, terrH, xzLim, yLimLo, yLimHi;
	DataHandler* terrain;

	glm::vec3 position;
	glm::vec3 lookAtPos;
	glm::vec3 up;

	glm::mat4 WTVMatrix;
	glm::mat4 VTPMatrix;

	bool unlocked;

	bool isInCollisionBox(glm::vec3 transVec, bool xz);
public:
	Camera(glm::vec3 startPos, int* initScreenW, int* initScreenH, int tH, int tW, int xzL, int yLL, int yLH, DataHandler* terr);

	void unlock();
	void rotate(char direction, GLfloat angle);
	void translate(GLfloat dx, GLfloat dy, GLfloat dz);
	void forward(GLfloat d);
	void strafe(GLfloat d);
	void jump(GLfloat d);

	void updateWTV();
	void updateVTP();
	void uploadCamData(GLuint program);

	glm::mat4* getWTV(); 
	glm::mat4* getVTP();
	glm::vec3* getPos();
	GLfloat getSpeed();
	GLfloat* getSpeedPtr();
	GLfloat getRotSpeed();
	GLfloat* getRotSpeedPtr();
	void toggleFrozen();

	/*************************************************************
	 * change_look_at_pos:
	 * Tar xrel från MouseMotionEvent och y som absolut koordinat
	 * width, height är storlek på nuvarande fönster
	 * Sätter look_at_pos därefter
	 * **********************************************************/
	void changeLookAtPos(int xrel, int yrel);
};

#endif
