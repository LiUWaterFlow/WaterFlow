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
        int program;
    public:
        Camera(int program, glm::mat4 *matrix);
        Camera();

	glm::vec3 position;
	glm::vec3 look_at_pos;
        glm::vec3 up;

        glm::mat4 *matrix;

        void rotate(char direction, float angle);
        void translate(float dx, float dy, float dz);
        void forward(float d);
        void strafe(float d);
        void update();
        float radius;
        void upload();
        void print_matrix(glm::mat4 m);

        /*************************************************************
         * change_look_at_pos:
         * Tar xrel från MouseMotionEvent och y som absolut koordinat
         * width, height är storlek på nuvarande fönster
         * Sätter look_at_pos därefter
         * **********************************************************/
        void change_look_at_pos(int xrel, int y, int width, int height);
};

#endif
