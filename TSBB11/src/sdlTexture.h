/// @file sdlTexture.h
/// @brief Loads image files and processes them into usable textures.

#ifndef SDLTEXTURE_H
#define SDLTEXTURE_H

#include "GL_utilities.h"
#include "SDL_image.h"
#include <string>

/// @class sdlTexture
/// @brief Stores an image as an SDL_Surface, and binds it as a usable texture.
///
/// Use the sdlTexture class to import image files into the project.
/// It is designed to be compatible with SDL2, and provides a texture ID
/// that can be used as (for instance) a terrain texture.
/// Supported file types are: BMP, GIF, JPEG, LBM, PCX, PNG, PNM, TGA, TIFF, WEBP, XCF, XPM, XV.
/// If there is an error loading the image file, gTextureID will be set to -1.
class sdlTexture
{
private:
	SDL_Surface* Surface; 			///< SDL_Surface used to store the image data.
	GLuint gTextureID;				///< Texture ID that the image is bound to.
	std::string texPath;			///< Path to the image file.
public:
	/// @brief Reads the image file and binds it to a texture ID.
	///
	/// tPath must be a valid path to an image file. After initialization,
	/// the texture ID can then be accessed using getTexID().
	/// @param tPath path to image file.
	/// @param gTexID texture ID. Can be initialized as 0 if ut's not pre-initialized ahead of time.
	/// @see getTexID()
	/// @todo Currently only designed to work on Windows.
	sdlTexture(std::string tPath, GLuint gTexID);

	/// @brief Getter for the texture ID bound to the loaded image. Will be -1 when image couldn't be loaded.
	/// @return texture ID bound to the image.
	GLuint getTexID();
};

#endif // SDLTEXTURE_H