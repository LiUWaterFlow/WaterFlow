#ifndef CAMERA_H
#define CAMERA_H

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <GLUT/glut.h>
#else
#ifdef  __linux__
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glext.h>

#else
#include "glew.h"
#endif
#endif


#ifndef M_PI
#define M_PI           3.14159265358979323846f
#endif

#define GLM_FORCE_RADIANS
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtx/rotate_vector.hpp"
#include "gtc/type_ptr.hpp"

class Camera {
private:
	// x används för att musen inte ska fastna i kanterna på
	// fönstret
	int x;
	int *screenW, *screenH;
	GLfloat speed, drawDistance;

	glm::vec3 position;
	glm::vec3 lookAtPos;
	glm::vec3 up;

	glm::mat4 WTVMatrix;
	glm::mat4 VTPMatrix;
public:
	Camera(glm::vec3 startPos, int* initScreenW, int* initScreenH);

	void rotate(char direction, GLfloat angle);
	void translate(GLfloat dx, GLfloat dy, GLfloat dz);
	void forward(GLfloat d);
	void strafe(GLfloat d);
	void jump(GLfloat d);

	void updateWTV();
	void updateVTP(int* newScreenW, int* newScreenH);
	void uploadCamData(GLuint program);

	glm::mat4* getWTV(); 
	glm::mat4* getVTP();
	glm::vec3* getPos();
	GLfloat getSpeed();
	GLfloat* getSpeedPtr();

	/*************************************************************
	 * change_look_at_pos:
	 * Tar xrel från MouseMotionEvent och y som absolut koordinat
	 * width, height är storlek på nuvarande fönster
	 * Sätter look_at_pos därefter
	 * **********************************************************/
	void changeLookAtPos(int xrel, int y);
};

#endif
