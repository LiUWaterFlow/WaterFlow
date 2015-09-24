#ifndef __TGA_LOADER__
#define __TGA_LOADER__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
#else
	#if defined(_WIN32)
		#include "glew.h"
	#endif
	#include <GL/gl.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __cplusplus
	#ifndef true
	#define true 1
	#endif

	#ifndef false
	#define false 0
	#endif

	#ifndef bool
	#define bool char
	#endif
#endif

typedef struct TextureData		// Create A Structure for .tga loading.
{
	GLubyte	*imageData;			// Image Data (Up To 32 Bits)
	GLuint	bpp;				// Image Color Depth In Bits Per Pixel.
	GLuint	width;				// Image Width
	GLuint	height;				// Image Height
	GLuint	w;				// Image Width "raw"
	GLuint	h;				// Image Height "raw"
	GLuint	texID;				// Texture ID Used To Select A Texture
	GLfloat	texWidth, texHeight;
} TextureData, *TextureDataPtr;					// Structure Name

bool LoadTGATexture(char *filename, TextureData *texture);
void LoadTGATextureSimple(char *filename, GLuint *tex);
void LoadTGASetMipmapping(bool active);
bool LoadTGATextureData(char *filename, TextureData *texture);

// Constants for SaveTGA
#define	TGA_ERROR_FILE_OPEN				-5
#define TGA_ERROR_READING_FILE			-4
#define TGA_ERROR_INDEXED_COLOR			-3
#define TGA_ERROR_MEMORY				-2
#define TGA_ERROR_COMPRESSED_FILE		-1
#define TGA_OK							 0

// Save functions
int SaveDataToTGA(char			*filename, 
			 short int		width, 
			 short int		height, 
			 unsigned char	pixelDepth,
			 unsigned char	*imageData);
void SaveTGA(TextureData *tex, char *filename);
void SaveFramebufferToTGA(char *filename, GLint x, GLint y, GLint w, GLint h);

#ifdef __cplusplus
}
#endif

#endif

