/// @file myDrawable.cpp
/// @brief Implementations of functions in myDrawable.h

#include "myDrawable.h"
#include "GL_utilities.h"

#include "Utilities.h"
#include "LoadTGA.h"

#include "gtx/transform.hpp"
#include "gtc/type_ptr.hpp"

myDrawable::myDrawable(GLuint program) 
: program(program) {}

SkyCube::SkyCube(GLuint program)
: myDrawable(program) {
	/* Initialize skycube */
	model = generateCube(10.0f);

	// Creating cubemap texture
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	TextureData texture1;
	memset(&texture1, 0, sizeof(texture1));
	TextureData texture2;
	memset(&texture2, 0, sizeof(texture2));
	TextureData texture3;
	memset(&texture3, 0, sizeof(texture3));
	TextureData texture4;
	memset(&texture4, 0, sizeof(texture4));
	TextureData texture5;
	memset(&texture5, 0, sizeof(texture5));
	TextureData texture6;
	memset(&texture6, 0, sizeof(texture6));

	LoadTGATextureData("resources/Skycube/Xn.tga", &texture1);
	LoadTGATextureData("resources/Skycube/Xp.tga", &texture2);
	LoadTGATextureData("resources/Skycube/Yn.tga", &texture3);
	LoadTGATextureData("resources/Skycube/Yp.tga", &texture4);
	LoadTGATextureData("resources/Skycube/Zn.tga", &texture5);
	LoadTGATextureData("resources/Skycube/Zp.tga", &texture6);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, texture1.width, texture1.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture1.imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, texture2.width, texture2.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture2.imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, texture3.width, texture3.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture3.imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, texture4.width, texture4.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture4.imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, texture5.width, texture5.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture5.imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, texture6.width, texture6.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture6.imageData);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void SkyCube::draw() {
	glUniform1i(glGetUniformLocation(program, "cube_texture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	DrawModel(model, program, "in_Position", NULL, NULL);
}

// ================================================================

HeightMap::HeightMap(GLuint drawProgram, GLuint* sizes, GLuint inputHeightBuffer, bool isBlue)
: myDrawable(drawProgram) {

	blue = isBlue;

	textureID = 0;

	dataWidth = sizes[0];
	dataHeight = sizes[1];
	numData = dataWidth * dataHeight;
	numIndices = (dataWidth - 1) * (dataHeight - 1) * 2 * 3;

	heightBuffer = inputHeightBuffer;
	
	initUpdate();

	initDraw();
}

void HeightMap::initUpdate() {

	glGenBuffers(4, drawBuffers);

	normalsProgram = compileComputeShader("src/shaders/normals.comp");

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawBuffers[3]); // Normals
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat)* 3 * numData, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	printError("init normals");

	heightMapProgram = compileComputeShader("src/shaders/heightMap.comp");

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawBuffers[0]); // Vertex positions
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * 3 * numData, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawBuffers[1]); // Texture coordinates
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * 2 * numData, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawBuffers[2]); // Indices
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLint) * numIndices, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	printError("init heightmap");
}

void HeightMap::initDraw() {
	// Initial one-time shader uploads.
	// Light information:
	glm::vec3 sunPos = { 0.58f, 0.58f, 0.58f }; // Since the sun is a directional source, this is the negative direction, not the position.
	bool sunIsDirectional = 1;
	float sunSpecularExponent = 50.0;
	glm::vec3 sunColor = { 1.0f, 1.0f, 1.0f };
	GLfloat sun_GLf[3] = { sunPos.x, sunPos.y, sunPos.z };

	glGenVertexArrays(1, &drawVAO);

	glUseProgram(program);
	glUniform3fv(glGetUniformLocation(program, "lightSourcePos"), 1, sun_GLf);
	glUniform1i(glGetUniformLocation(program, "isDirectional"), sunIsDirectional);
	glUniform1fv(glGetUniformLocation(program, "specularExponent"), 1, &sunSpecularExponent);
	GLfloat sunColor_GLf[3] = { sunColor.x, sunColor.y, sunColor.z };
	glUniform3fv(glGetUniformLocation(program, "lightSourceColor"), 1, sunColor_GLf);
	glUniform1i(glGetUniformLocation(program, "texUnit"), 0);

	printError("init draw uniforms");

	GLint posAttrib = glGetAttribLocation(program, "in_Position");
	GLint inNormAttrib = glGetAttribLocation(program, "in_Normal");
	GLint inTexAttrib = glGetAttribLocation(program, "in_TexCoord");

	glBindVertexArray(drawVAO);

	glBindBuffer(GL_ARRAY_BUFFER, drawBuffers[0]); //vertexBufferID
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)* 3, 0);

	printError("init draw1");

	//glBindBuffer(GL_ARRAY_BUFFER, drawBuffers[1]);//texBufferID
	//glEnableVertexAttribArray(inTexAttrib);
	//glVertexAttribPointer(inTexAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)* 2, 0);

	printError("init draw2");

	glBindBuffer(GL_ARRAY_BUFFER, drawBuffers[3]); //normalsBufferID
	glEnableVertexAttribArray(inNormAttrib);
	glVertexAttribPointer(inNormAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)* 3, 0);

	printError("init draw3");

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawBuffers[2]);//indicesBufferID

	printError("init draw4");

	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	printError("init draw");
}

void HeightMap::update() {

	glUseProgram(normalsProgram);

	glUniform2i(glGetUniformLocation(normalsProgram, "size"), dataWidth, dataHeight);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, heightBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, drawBuffers[3]);

	glDispatchCompute((GLuint)ceil((GLfloat)dataWidth / 16.0f), (GLuint)ceil((GLfloat)dataHeight / 16.0f), 1);

	printError("Update normals");

	glBindBuffersBase(GL_SHADER_STORAGE_BUFFER, 0, 3, drawBuffers);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, heightBuffer);

	glUseProgram(heightMapProgram);
	glUniform2i(glGetUniformLocation(heightMapProgram, "size"), dataWidth, dataHeight);
	glDispatchCompute((GLuint)ceil((GLfloat)dataWidth / 16.0f), (GLuint)ceil((GLfloat)dataHeight / 16.0f), 1);

	printError("Update vertices");
}

void HeightMap::draw() {
	glUseProgram(program);
	if (blue) {
		glUniform1i(glGetUniformLocation(program, "blue"), 1);
	} else {
		glUniform1i(glGetUniformLocation(program, "blue"), 0);
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glBindVertexArray(drawVAO);
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0L);

	printError("Draw Heightmap");
}