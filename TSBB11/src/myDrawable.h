/// @file myDrawable.h
/// @brief Contains classes with draw methods.

#ifndef MYDRAWABLE_H
#define MYDRAWABLE_H

#include "loadobj.h"
#include "glm.hpp"

#include "AntTweakBar.h"

#include <vector>

#ifdef _WINDOWS
	#include "sdlTexture.h"
#endif

/// @struct LightParams
/// @brief Light uniform
///
/// Used as a uniform buffer to avoid sending the lights more than once to the GPU.
/// Use this as an array to send multiple lights to the shader programs.
/// @note The order of the parameters is very important since the GPU needs to pack the arrays in 
/// a certain way. Please refer to OpenGL documentation for more information.
struct LightParams {
	glm::vec3 position;			///< Position of the light
	GLfloat isDirectional;		///< Is the light directional or not
	glm::vec3 color;			///< Color of the light
	GLfloat specularComponent;	///< Specular exponent of the light (should probably be part of material not light)
};

/// @enum textureUnits
/// @brief The texture unit used for different textures.
enum textureUnits {
	SKYBOX_TEXUNIT,
	TERRAINDATA_TEXUNIT,
	TERRAIN_FIRST_TEXUNIT,
	NOISE_TEXUNIT = TERRAIN_FIRST_TEXUNIT,
	DOTTED_TEXUNIT,
	GRASS_TEXUNIT,
	TOTAL_TEXTURES
};

/// @class myDrawable
/// @brief Contains functions for drawing and updating an entity
///
/// This is an abstact class that contains some basics for all drawable objects and is 
/// used as a base for more specific drawables.
class myDrawable {
protected:
	static GLuint lightBuffer;				///< The uniform buffer containing the lights
	static LightParams lightParam[2];		///< The CPU copy of the light information
	static GLuint texIDs[TOTAL_TEXTURES];	///< The texture ID's used in the program

	GLuint program; ///< The program that should be used to draw

public:
	/// @brief Simple constructor just sets the program
	myDrawable(GLuint program);

	/// @brief Use this to specify the way to draw the entity
	virtual void draw() = 0;

	/// @brief Use this to specify the way to update the entity
	virtual void update() = 0;

	/// @brief Set the lights used in the program
	static void setLights();

	/// @brief Set the textures used in the program
	static void setTextures(GLuint* size);
};


/// @class SkyCube
/// @brief Used as a skycube for the program
///
/// Probably only need one of these in the program.
class SkyCube : public myDrawable {
private:
	Model* model; ///< The model, a cube(!)

public:
	/// @brief Simple constructor just passes the program along.
	/// @param program The program to use when drawing the drawable
	SkyCube(GLuint program);

	/// @brief No update funtion for this one.
	///
	/// Could possibly be used to animate a moving sky or something awesome.
	virtual void update() {}

	/// @brief Draw call for the skycube.
	///
	/// Very simple just need to send the texture ID for the sky texture as uniform.
	virtual void draw();
};

/// @class HeightMap
/// @brief This class can be used to create models from height maps
///
/// Creates vertex data from a heightmap, generates vertices, normals, texture coordinates and
/// indices for drawing.
class HeightMap : public myDrawable {
 protected:
	GLuint drawBuffers[4];	///< The vertice data buffers in order: Vertex coordinate, Texture Coordinate, Normals, Incides
	GLuint heightBuffer;	///< Buffer with the height data information
	GLuint drawVAO;			///< Vertex Array for the drawable information
	GLuint texnum;			///< Currently used texture

	GLuint dataWidth;			///< Width of the data in the height map
	GLuint dataHeight;			///< Height of the data in the height map
	GLfloat dataTerrainHeight;	///< Diff between lowest and highest point in the height map
	GLuint numData;				///< Total number of vertices needed
	GLuint numIndices;			///< Total number of indices needed
	

	GLuint normalsProgram;		///< Compute shader for calculating the normals from a height map
	GLuint heightMapProgram;	///< Compute shader that generates the rest of the vertex data
	GLuint textureProgram;		///< Creates a texture with normal and height data, for use in other shaders where interpolation is important

	/// @brief Creates the buffers and shader programs
	///
	/// Init for creating buffers and shaders used for creating and/or updating the model vertice data.
	void initUpdate();

	/// @brief Sets the VAO pointers for drawing the model
	///
	/// Also uploads some of the static uniforms that are used for drawing.
	void initDraw();

public:
	/// @brief Simple contructor
	///
	/// Takes a few parameters for initial setup of the size of the height map data.
	/// @param drawProgram The initial program to use for drawing the model
	/// @param sizes Width and Height of the data
	/// @param maxHeight The height map maximum height (low assumed to be 0)
	/// @param inputHeightBuffer Buffer ID of the height map data
	HeightMap(GLuint drawProgram, GLuint* sizes, GLfloat maxHeight, GLuint inputHeightBuffer);
	
	/// @brief Calculates the new vertex data from the height map buffer
	///
	/// Calculates the normals first using sobel filters in x and z direction and then sets a
	/// height for y that makes the result look good (might need tuning). 
	/// After that the rest of the data is pretty straight forward to generate.
	virtual void update();

	/// @brief Draws the height field using the set draw program and texture.
	virtual void draw();

	/// @brief Generates the height data texture
	void generateHeightTexture();

	/// @brief Used by AntTweakBar to set which texture to use
	static void TW_CALL SetTextureCB(const void* value, void* clientData);

	/// @brief Used by AntTweakBar to get which texture is in use
	static void TW_CALL GetTextureCB(void* value, void* clientData);
};

/// @class Water
/// @brief Specialized version of height map that handles multiple drawing programs
///
/// Currently has the possibility of drawing two different programs and some special parameters.
/// @todo Could generalize this to the original height map class
class Water : public HeightMap {
private:
	GLfloat transparency;	///< Tranparancy of the water
	GLfloat maxDepth;		///< Depth drawn as black in the depth shader
	GLuint programs[2];		///< Program to toggle between (1 = normal, 2 = depth shader)
	GLint programToDraw;	///< Which program to use when drawing
	GLuint vaos[2];			///< Which VAO to use for drawing
	
	///@brief For water some additional parameters needs to be set
	///
	/// Creates extra VAO's for use with the other programs, still references the same buffers though
	/// but for depth drawing we done need normals.
	void initDepthProgram();

public:
	/// @brief Simple contructor
	///
	/// Takes a few parameters for initial setup of the size of the height map data.
	/// @param drawPrograms The initial program to use for drawing the model
	/// @param sizes Width and Height of the data
	/// @param maxHeight The height map maximum height (low assumed to be 0)
	/// @param inputHeightBuffer Buffer ID of the height map data
	Water(GLuint* drawPrograms, GLuint* sizes, GLfloat maxHeight, GLuint inputHeightBuffer);

	/// @brief Used by AntTweakBar to set which texture to use
	static void TW_CALL SetTransparencyCB(const void* value, void* clientData);
	/// @brief Used by AntTweakBar to get which texture is in use
	static void TW_CALL GetTransparencyCB(void* value, void* clientData);

	/// @brief Used by AntTweakBar to set which program should be used to draw
	static void TW_CALL SetDrawProgramCB(const void* value, void* clientData);
	/// @brief Used by AntTweakBar to get current program used for drawing
	static void TW_CALL GetDrawProgramCB(void* value, void* clientData);

	/// @brief Used by AntTweakBar to set max depth
	static void TW_CALL SetMaxDepthCB(const void* value, void* clientData);
	/// @brief Used by AntTweakBar to get max depth
	static void TW_CALL GetMaxDepthCB(void* value, void* clientData);
};





#endif // DRAWABLE_H
