/// @file myDrawable.cpp
/// @brief Implementations of functions in myDrawable.h

#include "myDrawable.h"
#include "GL_utilities.h"

#include "Utilities.h"
#include "LoadTGA.h"

#include "gtx/transform.hpp"
#include "gtx/rotate_vector.hpp"
#include "gtc/type_ptr.hpp"

myDrawable::myDrawable(GLuint program) 
: program(program) {}

GLuint myDrawable::lightBuffer;
LightParams myDrawable::lightParam[2];

void myDrawable::setLights() {
	// =========== Lights information ==========

	// Sun
	myDrawable::lightParam[0].position = { 0.58f, 0.58f, 0.58f }; // Since the sun is a directional source, this is the negative direction, not the position.
	myDrawable::lightParam[0].isDirectional = true;
	myDrawable::lightParam[0].color = { 1.0f, 1.0f, 1.0f };
	myDrawable::lightParam[0].specularComponent = 50.0f;

	// Calculating modified incoming light.

	// ---This should NOT be done here when inModel is not a plane. This should at that point be moved to watershader.frag.---
	// However, that is absolutely not trivial, since it requires information
	// about the surface fragment where the incident light refracted. If that
	// ends up being too complicated, this could be a decent approximation.
	float waterRefInd = 1.34451f;
	float airRefInd = 1.0f;
	glm::vec3 up = glm::vec3(0, 1, 0);
	glm::vec3 right = glm::cross(normalize(lightParam[0].position), up);
	// Snell's law.
	float theta1 = asinf(glm::length(right));
	float theta2 = asinf(airRefInd * sinf(theta1) / waterRefInd);
	glm::vec3 waterSunPos = glm::rotate(lightParam[0].position, theta1, right);
	waterSunPos = glm::rotate(waterSunPos, -theta2, right);
	// -----------------------------------------------------------------------------------------------------------------------

	// Sun under the surface
	myDrawable::lightParam[1].position = waterSunPos;
	myDrawable::lightParam[1].isDirectional = true;
	myDrawable::lightParam[1].color = { 1.0f, 1.0f, 1.0f };
	myDrawable::lightParam[1].specularComponent = 50.0f;


	glGenBuffers(1, &myDrawable::lightBuffer);

	glBindBuffer(GL_UNIFORM_BUFFER, myDrawable::lightBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LightParams), &myDrawable::lightParam, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, myDrawable::lightBuffer);
}

SkyCube::SkyCube(GLuint program, GLuint texUnit)
: myDrawable(program) {
	/* Initialize skycube */
	model = generateCube(10.0f);
	textureUnit = texUnit;

	// Creating cubemap texture
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0 + textureUnit);
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
	glUniform1i(glGetUniformLocation(program, "cube_texture"), textureUnit);

	DrawModel(model, program, "in_Position", NULL, NULL);
}

// ================================================================

HeightMap::HeightMap(GLuint drawProgram, GLuint* sizes, GLuint inputHeightBuffer, GLuint heightTexUnit, GLuint texUnit, GLuint texUnit1)
: myDrawable(drawProgram) {

	heightTextureUnit = heightTexUnit;
	textureUnit = texUnit;
	texnum = texUnit;
	textureUnit1 = texUnit1;
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	LoadTGATextureSimple("resources/grass.tga", &textureID);

	glActiveTexture(GL_TEXTURE0 + textureUnit1);
	LoadTGATextureSimple("resources/prickig.tga", &textureID1);

	glActiveTexture(GL_TEXTURE0 + 9);
	LoadTGATextureSimple("resources/noise.tga", &textureID2);

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

	glGenVertexArrays(1, &drawVAO);

	glUseProgram(program);

	printError("init draw uniforms");

	GLint posAttrib = glGetAttribLocation(program, "in_Position");
	GLint inNormAttrib = glGetAttribLocation(program, "in_Normal");
	GLint inTexAttrib = glGetAttribLocation(program, "in_TexCoord");

	glBindVertexArray(drawVAO);

	glBindBuffer(GL_ARRAY_BUFFER, drawBuffers[0]); //vertexBufferID
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)* 3, 0);

	printError("init draw1");

	glBindBuffer(GL_ARRAY_BUFFER, drawBuffers[1]); //texBufferID
	glEnableVertexAttribArray(inTexAttrib);
	glVertexAttribPointer(inTexAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)* 2, 0);

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

	printError("init draw5");

	glUniform1i(glGetUniformLocation(program, "terr_texUnit"), texnum);
	glUniform1i(glGetUniformLocation(program, "height_texUnit"), heightTextureUnit);
	glUniform1i(glGetUniformLocation(program, "sky_texUnit"), 2);
	glUniform2i(glGetUniformLocation(program, "size"), dataWidth, dataHeight);

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
	glUniform1i(glGetUniformLocation(program, "terr_texUnit"), texnum);
	glBindVertexArray(drawVAO);
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0L);

	printError("Draw Heightmap");
}

Water::Water(GLuint drawProgram, GLuint* sizes, GLuint inputHeightBuffer, GLuint terrNormalBuffer) 
: HeightMap(drawProgram, sizes, inputHeightBuffer) {
	transparency = 0.7f;

	glUniform1f(glGetUniformLocation(program, "transparency"), transparency);

	GLint terrNormAttrib = glGetAttribLocation(program, "in_terrNormal");
	glBindVertexArray(drawVAO);
	glBindBuffer(GL_ARRAY_BUFFER, terrNormalBuffer); //normalsBufferID
	glEnableVertexAttribArray(terrNormAttrib);
	glVertexAttribPointer(terrNormAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)* 3, 0);
	glBindVertexArray(0);


	glActiveTexture(GL_TEXTURE0 + 1);
	LoadTGATextureSimple("resources/grass.tga", &textureID);

	glActiveTexture(GL_TEXTURE0 + 4);
	LoadTGATextureSimple("resources/prickig.tga", &textureID1);

	glActiveTexture(GL_TEXTURE0 + 9);
	LoadTGATextureSimple("resources/noise.tga", &textureID2);
}


void TW_CALL Water::SetTransparencyCB(const void* value, void* clientData) {
	static_cast<Water*>(clientData)->transparency = *static_cast<const float*>(value);
	glUniform1f(glGetUniformLocation(static_cast<Water*>(clientData)->program, "transparency"), static_cast<Water*>(clientData)->transparency);
}

 void TW_CALL HeightMap::SetTextureCB(const void* value, void* clientData) {
 	static_cast<Water*>(clientData)->texnum = *static_cast<const GLuint*>(value);
 }


void TW_CALL Water::GetTransparencyCB(void* value, void* clientData) {
	*static_cast<float*>(value) = static_cast<Water*>(clientData)->transparency;
}

 void TW_CALL HeightMap::GetTextureCB(void* value, void* clientData) {
        *static_cast<int*>(value) = static_cast<Water*>(clientData)->texnum;
 }
