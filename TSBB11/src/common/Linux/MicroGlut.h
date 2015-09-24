#ifndef _MICROGLUT_
#define _MICROGLUT_

#ifdef __cplusplus
extern "C" {
#endif

// Same or similar to old GLUT calls
void glutMainLoop();
void glutCheckLoop();
void glutInit(int *argcp, char **argv);
void glutPostRedisplay();

void glutReshapeFunc(void (*func)(int width, int height));
void glutDisplayFunc(void (*func)(void));
void glutKeyboardFunc(void (*func)(unsigned char key, int x, int y));
void glutKeyboardUpFunc(void (*func)(unsigned char key, int x, int y));

void glutMouseFunc(void (*func)(int button, int state, int x, int y));
void glutPassiveMotionFunc(void (*func)(int x, int y));
void glutMotionFunc(void (*func)(int x, int y));

void glutInitWindowPosition (int x, int y);
void glutInitWindowSize (int width, int height);
void glutCreateWindow (char *windowTitle);

void glutSwapBuffers();

#define GLUT_ELAPSED_TIME		(700)
int glutGet(int type);

void glutInitDisplayMode(unsigned int mode);
void glutIdleFunc(void (*func)(void));

// Standard GLUT timer
void glutTimerFunc(int millis, void (*func)(int arg), int arg);
// Ingemar's version
void glutRepeatingTimerFunc(int millis); // Old name, will be removed
void glutRepeatingTimer(int millis);
// New call for polling the keyboard, good for games
char glutKeyIsDown(unsigned char c);
void glutWarpPointer( int x, int y );

void glutReshapeWindow(int width, int height);
void glutSetWindowTitle(char *title);
void glutInitContextVersion(int major, int minor);

/* Mouse buttons. */
#define GLUT_LEFT_BUTTON		0
// No support for middle yet
#define GLUT_MIDDLE_BUTTON		1
#define GLUT_RIGHT_BUTTON		2

/* Mouse button  state. */
#define GLUT_DOWN			0
#define GLUT_UP				1

// Only some modes supported
#define GLUT_STENCIL			32
//#define GLUT_MULTISAMPLE		128
//#define GLUT_STEREO			256
#define GLUT_RGB			0
#define GLUT_RGBA			GLUT_RGB
#define GLUT_SINGLE			0
#define GLUT_DOUBLE			2
#define GLUT_DEPTH			16


#ifdef __cplusplus
}
#endif


#endif
