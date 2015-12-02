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
GLuint myDrawable::texIDs[TOTAL_TEXTURES];

void myDrawable::setLights() {
	// =========== Lights information ==========

	// Sun
	myDrawable::lightParam[0].position = { 0.58f, 0.58f, 0.58f }; // Since the sun is a directional source, this is the negative direction, not the position.
	myDrawable::lightParam[0].isDirectional = 1.0f;
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
	myDrawable::lightParam[1].isDirectional = 1.0f;
	myDrawable::lightParam[1].color = { 1.0f, 1.0f, 1.0f };
	myDrawable::lightParam[1].specularComponent = 50.0f;


	glGenBuffers(1, &myDrawable::lightBuffer);

	glBindBuffer(GL_UNIFORM_BUFFER, myDrawable::lightBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LightParams) * 2, &myDrawable::lightParam, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, myDrawable::lightBuffer);

	printError("Create Light");
}

void myDrawable::setTextures(GLuint* size) {
	glGenTextures(TOTAL_TEXTURES, texIDs);

	// ===== Skybox Texture =====

	glActiveTexture(GL_TEXTURE0 + SKYBOX_TEXUNIT);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texIDs[SKYBOX_TEXUNIT]);

	TextureData tempTex;
	memset(&tempTex, 0, sizeof(tempTex));

	GLuint cubeSide[] = { GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_TEXTURE_CUBE_MAP_POSITIVE_Z };
	char *cubeImage[] = { "resources/Skycube/Xn.tga", "resources/Skycube/Xp.tga", "resources/Skycube/Yn.tga", "resources/Skycube/Yp.tga", "resources/Skycube/Zn.tga", "resources/Skycube/Zp.tga" };

	for (size_t i = 0; i < 6; i++) {
		LoadTGATextureData(cubeImage[i], &tempTex);

		glTexImage2D(cubeSide[i], 0, GL_RGB, tempTex.width, tempTex.height, 0, GL_RGB, GL_UNSIGNED_BYTE, tempTex.imageData);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// ===== Terrain Data Texture =====

	glActiveTexture(GL_TEXTURE0 + TERRAINDATA_TEXUNIT);
	glBindTexture(GL_TEXTURE_2D, texIDs[TERRAINDATA_TEXUNIT]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, size[0], size[1]);
	printError("Create textures1!");
	// ===== Grass Texture =====

	glActiveTexture(GL_TEXTURE0 + GRASS_TEXUNIT);
	LoadTGATextureSimple("resources/grass.tga", &texIDs[GRASS_TEXUNIT]);
	printError("Create textures2!");
	// ===== Dotted Texture =====

	glActiveTexture(GL_TEXTURE0 + DOTTED_TEXUNIT);
	LoadTGATextureSimple("resources/prickig.tga", &texIDs[DOTTED_TEXUNIT]);
	printError("Create textures3!");
	// ===== Noise Texture =====

	glActiveTexture(GL_TEXTURE0 + NOISE_TEXUNIT);
	LoadTGATextureSimple("resources/noise.tga", &texIDs[NOISE_TEXUNIT]);

	printError("Create textures!");
}


SkyCube::SkyCube(GLuint program)
: myDrawable(program) {
	/* Initialize skycube */
	model = generateCube(10.0f);
}

void SkyCube::draw() {
	glUniform1i(glGetUniformLocation(program, "cube_texture"), SKYBOX_TEXUNIT);

	DrawModel(model, program, "in_Position", NULL, NULL);
}

// ================================================================

HeightMap::HeightMap(GLuint drawProgram, GLuint* sizes, GLfloat maxHeight, GLuint inputHeightBuffer)
: myDrawable(drawProgram) {

	dataWidth = sizes[0];
	dataHeight = sizes[1];
	dataTerrainHeight = maxHeight;
	numData = dataWidth * dataHeight;
	numIndices = (dataWidth - 1) * (dataHeight - 1) * 2 * 3;

	heightBuffer = inputHeightBuffer;
	
	texnum = 0;

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

	textureProgram = compileComputeShader("src/shaders/textureData.comp");

	printError("init texturedata");
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

	glUniform1i(glGetUniformLocation(program, "height_texUnit"), TERRAINDATA_TEXUNIT);
	glUniform1i(glGetUniformLocation(program, "sky_texUnit"), SKYBOX_TEXUNIT);
	glUniform1i(glGetUniformLocation(program, "terr_texUnit"), texnum + TERRAIN_FIRST_TEXUNIT);
	glUniform3f(glGetUniformLocation(program, "size"), (float)dataWidth, dataTerrainHeight, (float)dataHeight);

	printError("init draw");
}

void HeightMap::genereteHeightTexture() {

	glUseProgram(textureProgram);

	glBindImageTexture(0, myDrawable::texIDs[TERRAINDATA_TEXUNIT], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	printError("Generate terrain texture data1");

	glUniform2i(glGetUniformLocation(textureProgram, "size"), dataWidth, dataHeight);
	glUniform1f(glGetUniformLocation(textureProgram, "maxHeight"), dataTerrainHeight);

	printError("Generate terrain texture data3");

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, heightBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, drawBuffers[3]);
	
	glDispatchCompute((GLuint)ceil((GLfloat)dataWidth / 16.0f), (GLuint)ceil((GLfloat)dataHeight / 16.0f), 1);
	
	printError("Generate terrain texture data");
	/*
	GLfloat* imagedata = new GLfloat[numData * 4];

	glActiveTexture(GL_TEXTURE0 + TERRAINDATA_TEXUNIT);
	glBindTexture(GL_TEXTURE_2D, myDrawable::texIDs[TERRAINDATA_TEXUNIT]);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, imagedata);

	for (size_t i = 0; i < 10 * 4; i+=4) {
		printf("Normal: %f, %f, %f  Height: %f\n", imagedata[i], imagedata[i + 1], imagedata[i + 2], imagedata[i + 3]);
	}
	*/
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
	glBindVertexArray(drawVAO);
	glUniform1i(glGetUniformLocation(program, "terr_texUnit"), texnum + TERRAIN_FIRST_TEXUNIT);
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0L);

	printError("Draw Heightmap");
}

Water::Water(GLuint drawProgram, GLuint* sizes, GLfloat maxHeight, GLuint inputHeightBuffer)
: HeightMap(drawProgram, sizes, maxHeight, inputHeightBuffer) {
	transparency = 0.7f;

	glUniform1f(glGetUniformLocation(program, "transparency"), transparency);
}


void TW_CALL Water::SetTransparencyCB(const void* value, void* clientData) {
	auto obj = static_cast<Water*>(clientData);
	obj->transparency = *static_cast<const float*>(value);
	glUniform1f(glGetUniformLocation(obj->program, "transparency"), obj->transparency);
}

 void TW_CALL HeightMap::SetTextureCB(const void* value, void* clientData) {
	static_cast<HeightMap*>(clientData)->texnum = *static_cast<const GLuint*>(value);
 }


void TW_CALL Water::GetTransparencyCB(void* value, void* clientData) {
	*static_cast<float*>(value) = static_cast<Water*>(clientData)->transparency;
}

 void TW_CALL HeightMap::GetTextureCB(void* value, void* clientData) {
	*static_cast<int*>(value) = static_cast<Water*>(clientData)->texnum;
 }
