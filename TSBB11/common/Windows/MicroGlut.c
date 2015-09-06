// Micro-GLUT, bare essentials

// By Ingemar Ragnemalm 2012
// I wrote this since GLUT seems not to have been updated to support
// creation of a 3.2 context on the Mac. You can use FreeGLUT
// which has this support, but I felt I wanted something without the old-style
// material, and that is small enough to include as a single source file.

// 120309 First Win32 version. Incomplete, lacks timers and more. Also, most settings are not supported.
// 130204 Tried to add GL3 and GLEW. No success.

// Forgotten for too long...
// 1508013 Running with all current utities and GLEW, with the psychedelic teapot demo!
// Timers and rescaling enabled, needs testing.
// 150817: Timers and rescaling runs fine!
// Menus and warp pointer are missing, but this looks good enough for "first beta version"!
// Tested mainly with the Psychedelic Teapot example.


#include <windows.h>
#include "glew.h"
#include <gl/gl.h>
#include "MicroGlut.h"

#ifndef _WIN32
	This line causes an error if you are not using Windows. It means that you are accidentally compiling the Windows version.
	Have a look at your paths.
#endif

// Vital internal variables

void (*gDisplay)(void);
void (*gReshape)(int width, int height);
void (*gKey)(unsigned char key, int x, int y);
void (*gKeyUp)(unsigned char key, int x, int y);
void (*gMouseMoved)(int x, int y);
void (*gMouseDragged)(int x, int y);
void (*gMouseFunc)(int button, int state, int x, int y);
unsigned int gContextInitMode = GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH;
void (*gIdle)(void);
char updatePending = 1;
char gRunning = 1;
int gContextVersionMajor = 0;
int gContextVersionMinor = 0;

// Prototype
static void checktimers();

// -----------

// Globals (was in GLViewDataPtr)
//NSOpenGLContext	*m_context;
float lastWidth, lastHeight;
//NSView *theView;

// Enable OpenGL

void EnableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC)
{
	PIXELFORMATDESCRIPTOR pfd;
	int format;
	int zdepth, sdepth;
#if defined WGL_CONTEXT_MAJOR_VERSION_ARB
// NOT tested because WGL_CONTEXT_MAJOR_VERSION_ARB is undefined on my computer
	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 2,
		WGL_CONTEXT_FLAGS_ARB, 0,
		0
	};
#endif	
	// get the device context (DC)
	*hDC = GetDC( hWnd );

	if (gContextInitMode & GLUT_DEPTH)
		zdepth = 32;
	else
		zdepth = 0;
	
	if (gContextInitMode & GLUT_STENCIL)
		sdepth = 32;
	else
		sdepth = 0;
	
	// set the pixel format for the DC
	// MUCH OF THIS SHOULD BE OPTIONAL (like depth and stencil above)!
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = zdepth;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat( *hDC, &pfd );
	SetPixelFormat( *hDC, format, &pfd );

#if defined WGL_CONTEXT_MAJOR_VERSION_ARB
// NOT tested because WGL_CONTEXT_MAJOR_VERSION_ARB is undefined on my computer
// Try to support new OpenGL!
	attribs[1] = gContextVersionMajor;
	attribs[3] = gContextVersionMinor;
 
    if(wglewIsSupported("WGL_ARB_create_context") == 1)
    {
		*hRC = wglCreateContextAttribsARB(*hDC,0, attribs);
		wglMakeCurrent(*hDC, *hRC);
	}
	else
	{	//It's not possible to make a GL 3.x context. Use the old style context (GL 2.1 and before)
		// create and enable the render context (RC)
		*hRC = wglCreateContext( *hDC );
		wglMakeCurrent( *hDC, *hRC );
	}
#else
		*hRC = wglCreateContext( *hDC );
		wglMakeCurrent( *hDC, *hRC );
#endif

}

// Disable OpenGL

void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( hRC );
	ReleaseDC( hWnd, hDC );
}





void glutPostRedisplay()
{
	updatePending = 1;
}

// ------------------ Main program ---------------------

//MGApplication *myApp;
//NSView *view;
//NSWindow *window;
static struct timeval timeStart;

void glutInit(int *argcp, char **argv)
{
}

int gWindowPosX = 10;
int gWindowPosY = 50;
int gWindowWidth = 400;
int gWindowHeight = 400;

void glutInitWindowPosition (int x, int y)
{
	gWindowPosX = x;
	gWindowPosY = y;
}
void glutInitWindowSize (int width, int height)
{
	gWindowWidth = width;
	gWindowHeight = height;
}

HWND hWnd;
HDC hDC;
HGLRC hRC;
HINSTANCE hInstance;

void glutCreateWindow(char *title)
{
	// Convert title to szTitle!
	#define MAX_LOADSTRING 100
	TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &title[0], strlen(title), NULL, 0);
    MultiByteToWideChar(CP_UTF8, 0, &title[0], strlen(title), &szTitle[0], size_needed);
	szTitle[size_needed] = 0; // Zero terminate

	// create main window
	hWnd = CreateWindow( 
		"GLSample" /*This is really wrong, should be 16-bit text like the title!*/, szTitle, 
		WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE | WS_OVERLAPPEDWINDOW, // WS_OVERLAPPEDWINDOW gives rescalable window
		0, 0, 256, 256,
		NULL, NULL, hInstance, NULL );
	
	// enable OpenGL for the window
	EnableOpenGL( hWnd, &hDC, &hRC );
}


void glutMainLoop()
{
	BOOL quit = 0;
	MSG msg;
	
	// program main loop
	while ( !quit )
	{
		
		// check for messages
		if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )  )
		{
			// handle or dispatch messages
			if ( msg.message == WM_QUIT ) 
			{
				quit = TRUE;
			} 
			else 
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		} 
		else 
		{
			if (updatePending)
			{
				gDisplay();
				updatePending = 0;
			}
			if (gIdle)
				gIdle();
			// TIMERS!
			checktimers();
		}
	}
}

// This won't work yet
//void glutCheckLoop()
//{
//}

//void glutTimerFunc(int millis, void (*func)(int arg), int arg)
//{
//}

// Added by Ingemar
//void glutRepeatingTimerFunc(int millis)
//{
//}

void glutDisplayFunc(void (*func)(void))
{
	gDisplay = func;
}

void glutReshapeFunc(void (*func)(int width, int height))
{
	gReshape = func;
}

void glutKeyboardFunc(void (*func)(unsigned char key, int x, int y))
{
	gKey = func;
}

void glutKeyboardUpFunc(void (*func)(unsigned char key, int x, int y))
{
	gKeyUp = func;
}

void glutPassiveMotionFunc(void (*func)(int x, int y))
{
	gMouseMoved = func;
}

void glutMotionFunc(void (*func)(int x, int y))
{
	gMouseDragged = func;
}

void glutMouseFunc(void (*func)(int button, int state, int x, int y))
{
	gMouseFunc = func;
}

// You can safely skip this
void glutSwapBuffers()
{
 	SwapBuffers(hDC);
}

int glutGet(int type)
{
//	struct timeval tv;
	
//	gettimeofday(&tv, NULL);
//	return (tv.tv_usec - timeStart.tv_usec) / 1000 + (tv.tv_sec - timeStart.tv_sec)*1000;

	return GetTickCount();
}

void glutInitDisplayMode(unsigned int mode)
{
	gContextInitMode = mode;
}

void glutIdleFunc(void (*func)(void))
{
//	printf('WARNING! Idle not yet implemented. Use timers instead.\n');
	gIdle = func;
	glutRepeatingTimer(10);
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	
	switch (message)
	{
    case WM_LBUTTONUP:
		if (gMouseFunc != NULL)
			gMouseFunc(GLUT_LEFT_BUTTON, GLUT_UP, x, y);
        break;

    case WM_LBUTTONDOWN:
		if (gMouseFunc != NULL)
			gMouseFunc(GLUT_LEFT_BUTTON, GLUT_DOWN, x, y);
     break;

    case WM_RBUTTONUP:
		if (gMouseFunc != NULL)
			gMouseFunc(GLUT_RIGHT_BUTTON, GLUT_UP, x, y);
        break;

    case WM_RBUTTONDOWN:
		if (gMouseFunc != NULL)
			gMouseFunc(GLUT_RIGHT_BUTTON, GLUT_DOWN, x, y);
     break;

    case WM_MOUSEMOVE:
		if (gMouseMoved != NULL)
			gMouseMoved(x, y);
    break;

	case WM_CREATE:
		return 0;
		
	case WM_CLOSE:
		PostQuitMessage( 0 );
		return 0;
		
	case WM_DESTROY:
		return 0;
		
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
			return 0;
		}
		else
		if (gKey != NULL)
		{
			gKey(wParam, 0, 0); // TO DO: x and y
		}
		return 0;

	case WM_KEYUP:
		break;
	
	case WM_SIZE:
		if (gReshape != NULL)
			gReshape(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_PAINT: // Don't have Windows fighting us while resize!
		gDisplay();
		break;
	case WM_ERASEBKGND:
		return 1;
		break;
	default:
		return DefWindowProc( hWnd, message, wParam, lParam );
	}

	return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int iCmdShow)
{
	WNDCLASS wc;
	HWND hWnd = NULL;
	HDC hDC = NULL;
	HGLRC hRC = NULL;
//	MSG msg;
	BOOL quit = FALSE;
	float theta = 0.0f;
	
	// register window class
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "GLSample";
	RegisterClass( &wc );
	
	main();
	
	// shutdown OpenGL
	DisableOpenGL( hWnd, hDC, hRC );
	
	// destroy the window explicitly
	DestroyWindow( hWnd );
	
	return 0;
//	return msg.wParam;	
}










// NOTE: The timer is not designed with any multithreading in mind!
typedef struct TimerRec
{
	int arg;
	int time;
	int repeatTime;
	void (*func)(int arg);
	char repeating;
	struct TimerRec *next;
	struct TimerRec *prev;
} TimerRec;

TimerRec *gTimers = NULL;

void glutTimerFunc(int millis, void (*func)(int arg), int arg)
{
	TimerRec *t	= (TimerRec *)malloc(sizeof(TimerRec));
	t->arg = arg;
	t->time = millis + glutGet(GLUT_ELAPSED_TIME);
	t->repeatTime = 0;
	t->repeating = 0;
	t->func = func;
	t->next = gTimers;
	t->prev = NULL;
	if (gTimers != NULL)
		gTimers->prev = t;
	gTimers = t;
}

// Added by Ingemar
void glutRepeatingTimer(int millis)
{
	TimerRec *t	= (TimerRec *)malloc(sizeof(TimerRec));
	t->arg = 0;
	t->time = millis + glutGet(GLUT_ELAPSED_TIME);
	t->repeatTime = millis;
	t->repeating = 1;
	t->func = NULL;
	t->next = gTimers;
	t->prev = NULL;
	if (gTimers != NULL)
		gTimers->prev = t;
	gTimers = t;
}

// From http://stackoverflow.com/questions/5801813/c-usleep-is-obsolete-workarounds-for-windows-mingw
void usleep(__int64 usec) 
{ 
    HANDLE timer; 
    LARGE_INTEGER ft; 

    ft.QuadPart = -(10*usec); // Convert to 100 nanosecond interval, negative value indicates relative time

    timer = CreateWaitableTimer(NULL, TRUE, NULL); 
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0); 
    WaitForSingleObject(timer, INFINITE); 
    CloseHandle(timer); 
}

static void checktimers()
{
	if (gTimers != NULL)
	{
		TimerRec *t, *firethis = NULL;
		int now = glutGet(GLUT_ELAPSED_TIME);
		int nextTime = now + 1000; // Distant future, 1 second
		t = gTimers;
		for (t = gTimers; t != NULL; t = t->next)
		{
			if (t->time < nextTime) nextTime = t->time; // Time for the next one
			if (t->time < now) // See if this is due to fire
			{
				firethis = t;
			}
		}
		if (firethis != NULL)
		{
		// Fire the timer
			if (firethis->func != NULL)
				firethis->func(firethis->arg);
			else
				glutPostRedisplay();
		// Remove the timer if it was one-shot, otherwise update the time
			if (firethis->repeating)
			{
				firethis->time = now + firethis->repeatTime;
			}
			else
			{
				if (firethis->prev != NULL)
					firethis->prev->next = firethis->next;
				else
					gTimers = firethis->next;
                if (firethis->next != NULL)
					firethis->next->prev = firethis->prev;
				free(firethis);
			}
		}
		// Otherwise, sleep until any timer should fire
        if (!updatePending)
			if (nextTime > now)
            {
		usleep((nextTime - now)*1000);
            }
	}
    else
// If no timer and no update, sleep a little to keep CPU load low
        if (!updatePending)
            usleep(10);
}

void glutInitContextVersion(int major, int minor)
{
	gContextVersionMajor = major;
	gContextVersionMinor = minor;
}
