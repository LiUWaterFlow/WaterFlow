#ifndef _GL_UTILITIES_
#define _GL_UTILITIES_

#ifdef __cplusplus
extern "C" {
#endif

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

char* readFile(const char *file);
void printError(const char *functionName);
GLuint loadShaders(const char *vertFileName, const char *fragFileName);
GLuint loadShadersG(const char *vertFileName, const char *fragFileName, const char *geomFileName);
GLuint loadShadersGT(const char *vertFileName, const char *fragFileName, const char *geomFileName,
						const char *tcFileName, const char *teFileName);
void dumpInfo(void);

void initKeymapManager();
char keyIsDown(unsigned char c);

// FBO support

//------------a structure for FBO information-------------------------------------
typedef struct
{
	GLuint texid;
	GLuint fb;
	GLuint rb;
	GLuint depth;
	int width, height;
} FBOstruct;

FBOstruct *initFBO(int width, int height, int int_method);
FBOstruct *initFBO2(int width, int height, int int_method, int create_depthimage);
FBOstruct *initFBO3(int width, int height, void* data);
FBOstruct *initFBO4(int width, int height, void* data);
FBOstruct *initFBO5(int width, int height, void* data);
void printShaderInfoLog(GLuint obj, const char *fn);
void printProgramInfoLog(GLuint obj, const char *vfn, const char *ffn,
					const char *gfn, const char *tcfn, const char *tefn);
void releaseFBO(FBOstruct* fbo);
void releaseFBO2(FBOstruct* fbo, int keepTexure);
void useFBO(FBOstruct *out, FBOstruct *in1, FBOstruct *in2);
void updateScreenSizeForFBOHandler(int w, int h); // Temporary workaround to inform useFBO of screen size changes

#ifdef __cplusplus
}
#endif

#endif
