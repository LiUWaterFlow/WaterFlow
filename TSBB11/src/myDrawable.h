/// @file drawable.h
/// @brief Contains classes with draw methods.

#ifndef MYDRAWABLE_H
#define MYDRAWABLE_H

#include "loadobj.h"
#include "glm.hpp"

#include "AntTweakBar.h"

#include <vector>

struct LightParams {
	glm::vec3 position;
	bool isDirectional;
	glm::vec3 color;
	float specularComponent;
};

class myDrawable {
private:
	

protected:
	glm::mat4 MTWMatrix;
	GLuint program;

public:
	static GLuint lightBuffer;
	static LightParams lightParam[2];

	myDrawable(GLuint program);
	virtual void draw() = 0;
	virtual void update() = 0;

	static void setLights();

};

class SkyCube : public myDrawable {
private:
	Model* model;
	GLuint textureUnit;
	GLuint textureID;

public:
	SkyCube(GLuint program, GLuint texUnit = 2);
	virtual void update() {}
	virtual void draw();
};

class HeightMap : public myDrawable {
 protected:
	GLuint drawBuffers[4];
	GLuint heightBuffer;
	GLuint drawVAO;
	GLuint heightTextureUnit;
	GLuint heightTextureID;
	GLuint textureUnit;
	GLuint textureUnit1;
	GLuint textureID;
	GLuint textureID1;
	GLuint textureID2;

	GLuint dataWidth, dataHeight, numData, numIndices;

	GLuint normalsProgram, heightMapProgram;

	void initUpdate();
	void initDraw();

public:

	HeightMap(GLuint drawProgram, GLuint* sizes, GLuint inputHeightBuffer, GLuint heightTexUnit = 3, GLuint texUnit = 1 , GLuint texUnit1 = 5);
	GLuint texnum;
	virtual void update();
	virtual void draw();
	static void TW_CALL SetTextureCB(const void* value, void* clientData);
	static void TW_CALL GetTextureCB(void* value, void* clientData);
	GLuint getNormalBuffer() { return drawBuffers[3]; }
};

class Water : public HeightMap {
private:
	GLfloat transparency;

public:
	Water(GLuint drawProgram, GLuint* sizes, GLuint inputHeightBuffer, GLuint terrNormalBuffer);

	static void TW_CALL SetTransparencyCB(const void* value, void* clientData);
	static void TW_CALL GetTransparencyCB(void* value, void* clientData);

};





#endif // DRAWABLE_H
