/// @file sdlTexture.cpp
/// @brief Implementations of functions sdlTexture.h

#include "sdlTexture.h"

// ===== Constructor

sdlTexture::sdlTexture(std::string tPath)
{
	// Initialization of texture ID.
	gTextureID = 0;
	texPath = tPath;
	// The ifdef below breaks this class for non-Windows, but is as of now needed in order to run branch on Linux.
#ifdef _WINDOWS
	Surface = IMG_Load(texPath.c_str());
#endif

	glGenTextures(1, &gTextureID);
	glBindTexture(GL_TEXTURE_2D, gTextureID);

	int Mode = GL_RGB;

	if (Surface->format->BytesPerPixel == 4) {
		Mode = GL_RGBA;
	}
	// Bind image to the texture.
	glTexImage2D(GL_TEXTURE_2D, 0, Mode, Surface->w, Surface->h, 0, Mode, GL_UNSIGNED_BYTE, Surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

// ===== Getter =====

GLuint sdlTexture::getTexID()
{
	return gTextureID;
}