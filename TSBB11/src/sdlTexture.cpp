/// @file sdlTexture.cpp
/// @brief Implementations of functions sdlTexture.h

#include "sdlTexture.h"

// ===== Constructor =====

sdlTexture::sdlTexture(std::string tPath, GLuint gTexID)
{
	gTextureID = gTexID;
	texPath = tPath;
	// The ifdef below breaks this class for non-Windows, but is as of now needed in order to run branch on Linux.
#ifdef _WINDOWS
	try
	{
		Surface = IMG_Load(texPath.c_str());
	}
	catch (const char* msg)
	{
		printError("Invalid terrain image path!");
		printError(msg);
	}
#endif
	if (Surface != NULL)
	{
		glGenTextures(1, &gTextureID);
		glBindTexture(GL_TEXTURE_2D, gTextureID);

		int Mode = GL_RGB;

		if (Surface->format->BytesPerPixel == 4) {
			Mode = GL_RGBA;
		}
		// Bind image to the texture.
		glTexImage2D(GL_TEXTURE_2D, 0, Mode, Surface->w, Surface->h, 0, Mode, GL_UNSIGNED_BYTE, Surface->pixels);
	}
	else
	{
		// If any error was encountered when loading the image,
		// such as incorrect file or incorrect path, gTextureID
		// will be set to -1 to indicate such an error.
		gTextureID = -1;
	}
}

// ===== Getter =====

GLuint sdlTexture::getTexID()
{
	return gTextureID;
}