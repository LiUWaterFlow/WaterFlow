/// @file drawable.h
/// @brief Contains classes with draw methods.

#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "loadobj.h"
#include "glm.hpp"

#include <vector>

class Drawable {
protected:
	glm::mat4 MTWMatrix;
	GLuint program;

public:
	Drawable(GLuint program);
	virtual void draw() = 0;
};

class SkyCube : public Drawable {
private:
	Model* model;
	GLuint textureID;

public:
	SkyCube(GLuint program);
	virtual void draw();
};

class Terrain : public Drawable {
private:
	std::vector<Model*>* model;
	glm::mat3 inverseNormalMatrixTrans;

public:
	Terrain(GLuint program, std::vector<Model*>*, glm::vec3 scale);
	virtual void draw();
};


#endif // DRAWABLE_H