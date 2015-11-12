// Written by Gustav Svensk, acquired from
// https://github.com/DrDante/TSBK03Project/
// with permission, 2015-09-24.
#include "SDL_util.h"
#include <stdio.h>

SDL_Window* screen ;
SDL_GLContext glcontext;
int bpp;

void (*ptdisplay)(void);
void (*handle_event)(SDL_Event event);


void set_sdl_display_func(void (*display_func)(void))
{
	ptdisplay = display_func;
	if(ptdisplay == NULL){
		fprintf(stderr, "Error setting display function\n");
	}
}

void set_event_handler(void (*event_func)(SDL_Event event))
{
    handle_event = event_func;
}

void exit_prog(int value)
{
    SDL_GL_DeleteContext(glcontext);
    SDL_Quit();
    exit(value);
}

void terminate_prog( void )
{
	exit_prog(0);
}

void resize_window(SDL_Event event)
{
	SDL_SetWindowSize(screen, event.window.data1, event.window.data2);

	if(screen == NULL){
		fprintf(stderr, "Error resizing window: %s", SDL_GetError());
		exit_prog(1);
	}
}

void inf_loop()
{
	SDL_Event event;

	while(1){
		while(SDL_PollEvent(&event)){
			(*handle_event)(event);
		}
	}
}

void swap_buffers()
{
    SDL_GL_SwapWindow(screen);
}

void get_window_size(int *w, int*h)
{
    SDL_GetWindowSize(screen, w, h);
}

void init_SDL(const char* title, int width, int height)
{
	int flags = 0;
	if(SDL_Init(SDL_INIT_VIDEO) != 0){
		fprintf(stderr, "Failed to initialise SDL: %s", SDL_GetError());
		exit_prog(1);
	}


	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);

#ifdef __APPLE__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);


	flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
	screen = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);


	glcontext = SDL_GL_CreateContext(screen);


	if(screen == 0){
		fprintf(stderr, "Failed to set Video Mode: %s", SDL_GetError());
		exit_prog(1);
	}


#ifndef __APPLE__
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glOrtho(0, width, 0, height, -1, 1);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
#endif

	atexit(terminate_prog);
}
