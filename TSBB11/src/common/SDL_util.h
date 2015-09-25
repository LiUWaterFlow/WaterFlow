#ifndef SDL_UTIL_H
#define SDL_UTIL_H

#ifdef __APPLE__
	//#include <GLUT/glut.h>
	#include <OpenGL/gl3.h>
#else
	#include <GL/gl.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __APPLE__
	#include <SDL2/SDL.h>
	//#include "MicroGlut.h"
#else
	//#include "MicroGlut.h"
	##include "Windows/sdl2/SDL.h"
#endif


/******************************************************************************
 * Koder för egna event, såsom timers och liknande.
 * Används för att indentifiera ett specifikt event.
 *
 *****************************************************************************/

/******************************************************************************
 * Sätt funktionen som anropas i inf_loop varje gång skärmen skall uppdateras
 * Inparametrar::	void (*display_func)(void)	pekare till funktionen.
 * 			får INTE returnera något
 * Returnerar:		void
 *****************************************************************************/
void set_sdl_display_func(void (*display_func)(void));

/******************************************************************************
 * Sätt lämplig funktion för att hantera SD_event event
 *****************************************************************************/

void set_event_handler(void (*event_func)(SDL_Event event));

/******************************************************************************
 * Initierar SDLs system så att allt bör kunna fungera
 * Inparametrar:	const char* title		Titel till fönstret
 *			int width			Bredd på fönstret
 *			int height			Höjd på fönstret
 * Returnerar:		-
 *****************************************************************************/
void init_SDL(const char* title, int width, int height);

/******************************************************************************
 * oändlig loop som väntar på events och hanterar dessa. Uppdaterar skärmen.
 * Inparametrar:	-
 * Returnerar:		-
 *****************************************************************************/
void inf_loop();

void get_window_size(int* w, int* h);

/*****************************************************************************
 * Byt buffer, ersätter glutSwapBuffers
 * ***************************************************************************/
void swap_buffers();

void resize_window(SDL_Event event);

//void handle_keypress(SDL_Event event);

void exit_prog(int);

//void handle_userevent(SDL_Event event);

#ifdef __cplusplus
}
#endif

#endif //SDL_UTIL_H
