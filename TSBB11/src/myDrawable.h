/// @file drawable.h
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

struct LightParams {
	glm::vec3 position;
	GLfloat isDirectional;
	glm::vec3 color;
	GLfloat specularComponent;
};

enum textureUnits {
	SKYBOX_TEXUNIT,
	TERRAINDATA_TEXUNIT,
	TERRAIN_FIRST_TEXUNIT,
	GRASS_TEXUNIT = TERRAIN_FIRST_TEXUNIT,
	DOTTED_TEXUNIT,
	NOISE_TEXUNIT,
	TOTAL_TEXTURES
};


class myDrawable {
protected:
	GLuint program;

	static GLuint lightBuffer;
	static LightParams lightParam[2];
	static GLuint texIDs[TOTAL_TEXTURES];

public:
	myDrawable(GLuint program);
	virtual void draw() = 0;
	virtual void update() = 0;

	static void setLights();
	static void setTextures(GLuint* size);
};

class SkyCube : public myDrawable {
private:
	Model* model;

public:
	SkyCube(GLuint program);
	virtual void update() {}
	virtual void draw();
};

class HeightMap : public myDrawable {
 protected:
	GLuint drawBuffers[4];
	GLuint heightBuffer;
	GLuint drawVAO;
	GLuint texnum;

	GLuint dataWidth, dataHeight, numData, numIndices;
	GLfloat dataTerrainHeight;

	GLuint normalsProgram, heightMapProgram, textureProgram;

	void initUpdate();
	void initDraw();

public:

	HeightMap(GLuint drawProgram, GLuint* sizes, GLfloat maxHeight, GLuint inputHeightBuffer);
	
	virtual void update();
	virtual void draw();

	static void TW_CALL SetTextureCB(const void* value, void* clientData);
	static void TW_CALL GetTextureCB(void* value, void* clientData);

	void genereteHeightTexture();

};

class Water : public HeightMap {
private:
	GLfloat transparency;
	GLfloat maxDepth;
	GLuint programs[2];
	GLint programToDraw;
	GLuint vaos[2];
	
	void initDepthProgram();

public:
	Water(GLuint* drawPrograms, GLuint* sizes, GLfloat maxHeight, GLuint inputHeightBuffer);

	static void TW_CALL SetTransparencyCB(const void* value, void* clientData);
	static void TW_CALL GetTransparencyCB(void* value, void* clientData);

	static void TW_CALL SetDrawProgramCB(const void* value, void* clientData);
	static void TW_CALL GetDrawProgramCB(void* value, void* clientData);

	static void TW_CALL SetMaxDepthCB(const void* value, void* clientData);
	static void TW_CALL GetMaxDepthCB(void* value, void* clientData);
};





#endif // DRAWABLE_H
