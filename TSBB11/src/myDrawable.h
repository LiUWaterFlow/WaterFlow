/// @file drawable.h
/// @brief Contains classes with draw methods.

#ifndef MYDRAWABLE_H
#define MYDRAWABLE_H

#include "loadobj.h"
#include "glm.hpp"

#include <vector>

class myDrawable {
protected:
	glm::mat4 MTWMatrix;
	GLuint program;

public:
	myDrawable(GLuint program);
	virtual void draw() = 0;
	virtual void update() = 0;
};

class SkyCube : public myDrawable {
private:
	Model* model;
	GLuint textureID;

public:
	SkyCube(GLuint program);
	virtual void update() {}
	virtual void draw();
};

class HeightMap : public myDrawable {
private:
	GLuint drawBuffers[4];
	GLuint heightBuffer;
	GLuint drawVAO;
	GLuint textureID; // Currently not used
	bool blue;

	GLuint dataWidth, dataHeight, numData, numIndices;

	GLuint normalsProgram, heightMapProgram;

	void initUpdate();
	void initDraw();

public:
	HeightMap(GLuint drawProgram, GLuint* sizes, GLuint inputHeightBuffer, bool isBlue = false);

	virtual void update();
	virtual void draw();
};



#endif // DRAWABLE_H
