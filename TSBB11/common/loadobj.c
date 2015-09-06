// LoadOBJ
// by Ingemar Ragnemalm 2005, 2008
// Developed with CodeWarrior and Lightweight IDE on Mac OS/Mac OSX

// Assumes that gcc is set to -std=c99 (No, not any more.)

// Extended version with LoadModelPlus
// 120913: Revised LoadModelPlus/DrawModel by Jens.
// Partially corrected formatting. (It is a mess!)
// 130227: Error reporting in DrawModel
// 130422: Added ScaleModel

#include "loadobj.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PI 3.141592


typedef struct Mesh
{
	GLfloat	*vertices;
	int		vertexCount;
	GLfloat	*vertexNormals;
	int		normalsCount; // Same as vertexCount for generated normals
	GLfloat	*textureCoords;
	int		texCount;
	
	int		*coordIndex;
	int		*normalsIndex;
	int		*textureIndex;
	int		coordCount; // Number of indices in each index struct
	
	int		*triangleCountList;
	int		**vertexToTriangleTable;
	
	GLfloat radius; // Enclosing sphere
	GLfloat radiusXZ; // For cylindrical tests
} Mesh, *MeshPtr;



#define vToken				1
#define vnToken			 2
#define vtToken			 3
#define kReal				 4
#define kInt					5
#define tripletToken	6
#define fToken				7
#define crlfToken		 8
#define kEOF					9
#define kUnknown		 10


static FILE *fp;

static int intValue[3];
static float floatValue[3];
static int vertCount, texCount, normalsCount, coordCount;

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif
#ifndef bool
#define bool char
#endif

static bool hasPositionIndices;
static bool hasNormalIndices;
static bool hasTexCoordIndices;

static bool atLineEnd; // Helps SkipToCRLF


static void OBJGetToken(int * tokenType)
{
	char c;
	char s[255];
	int i;
	
	// 1. skip space. Check for #, skip line when found
	c = getc(fp);
	while (c == 32 || c == 9 || c == '#')
		{
			while (c == '#')
	while (c != 13 && c != 10 && c != EOF)
		c = getc(fp); // Skip comment
			c = getc(fp);
		}
	
	// Inspect first character. Bracket, number, other?
	
	if (c == 13 || c == 10)
		{
			*tokenType = crlfToken;
			//		while (c == 13 && c == 10)
			//				c = getc(fp);
		}
	else
		if ((c >= '0' && c <= '9') || c == '-' || c == '.') // Numerical value
			{
	*tokenType = kInt;
	i = 0;
	while (c != 13 && c != 10 && c != 32 && c != 9 && c != '/' && c != EOF)
		{
			if (c == '.' || c == 'E')
				*tokenType = kReal;
			s[i++] = c;
			c = getc(fp);
		}
	s[i] = 0;
	sscanf(s, "%f", &floatValue[0]);
	sscanf(s, "%d", &intValue[0]);
	// Check for /
	if (c == '/') // parse another number
		{
			c = getc(fp);
			i = 0;
			while (c != 13 && c != 10 && c != 32 && c != 9 && c != '/' && c != EOF)
				{
		s[i++] = c;
		c = getc(fp);
				}
			s[i] = 0;
			
			if (i == 0)
				{
		floatValue[1] = -1;
		intValue[1] = -1;
				}
			else
				{
		sscanf(s, "%f", &floatValue[1]);
		sscanf(s, "%d", &intValue[1]);
				}
			*tokenType = tripletToken;
		}
	if (c == '/') // parse one more number
		{
			c = getc(fp);
			i = 0;
			while (c != 13 && c != 10 && c != 32 && c != 9 && c != '/' && c != EOF)
				{
		s[i++] = c;
		c = getc(fp);
				}
			s[i] = 0;

			if (i == 0)
				{
		floatValue[2] = -1;
		intValue[2] = -1;
				}
			else
				{
		sscanf(s, "%f", &floatValue[2]);
		sscanf(s, "%i", &intValue[2]);
				}
			*tokenType = tripletToken;
		}
		}
		else
			if (c == EOF)
	{
		*tokenType = kEOF;
	}
			else // Other
	{
		i = 0;
		while (c != 13 && c != 10 && c != 32 && c != 9 && c != EOF)
			{
				s[i++] = c;
				c = getc(fp);
			}
		s[i] = 0;
		
		*tokenType = kUnknown;
		// Compare string to symbols
		
		if (strcmp(s, "v") == 0)
			*tokenType = vToken;
		if (strcmp(s, "vn") == 0)
			*tokenType = vnToken;
		if (strcmp(s, "vt") == 0)
			*tokenType = vtToken;
		if (strcmp(s, "f") == 0)
			*tokenType = fToken;
	}
	atLineEnd = (c == 13 || c == 10);
} // ObjGetToken

static void SkipToCRLF()
{
	char c = 0;
	
	if (!atLineEnd)
		while (c != 10 && c != 13 && c != EOF)
			c = getc(fp);
	//	while (tokenType != crlfToken && tokenType != kEOF)
	//		OBJGetToken(&tokenType);
}

//static void SkipToCRLF(int tokenType)
//{
//	while (tokenType != crlfToken && tokenType != kEOF)
//		OBJGetToken(&tokenType);
//}

static void ReadOneVertex(MeshPtr theMesh)
{
	GLfloat x, y, z;
	int tokenType;

	// Three floats expected
	OBJGetToken(&tokenType);
	if (tokenType == kInt || tokenType == kReal)
		x = floatValue[0];
	OBJGetToken(&tokenType);
	if (tokenType == kInt || tokenType == kReal)
		y = floatValue[0];
	OBJGetToken(&tokenType);
	if (tokenType == kInt || tokenType == kReal)
		z = floatValue[0];
	SkipToCRLF();
	
	// Write to array if it exists
	if (theMesh->vertices != NULL)
		{
			theMesh->vertices[vertCount++] = x;
			theMesh->vertices[vertCount++] = y;
			theMesh->vertices[vertCount++] = z;
		}
	else
		vertCount = vertCount + 3;
}

static void ReadOneTexture(MeshPtr theMesh)
{
	int tokenType;
	GLfloat s, t;

	// Two floats expected
	OBJGetToken(&tokenType);
	if (tokenType == kInt || tokenType == kReal)
		s = floatValue[0];
	OBJGetToken(&tokenType);
	if (tokenType == kInt || tokenType == kReal)
		t = floatValue[0];
	SkipToCRLF();
	
	// Write to array if it exists
	if (theMesh->textureCoords != NULL)
		{
			theMesh->textureCoords[texCount++] = s;
			theMesh->textureCoords[texCount++] = t;
	  }
	else
		texCount = texCount + 2;
}

static void ReadOneNormal(MeshPtr theMesh)
{
	int tokenType;
	GLfloat x, y, z;

	// Three floats expected
	OBJGetToken(&tokenType);
	if (tokenType == kInt || tokenType == kReal)
		x = floatValue[0];
	OBJGetToken(&tokenType);
	if (tokenType == kInt || tokenType == kReal)
		y = floatValue[0];
	OBJGetToken(&tokenType);
	if (tokenType == kInt || tokenType == kReal)
		z = floatValue[0];
	SkipToCRLF();
	
	// Write to array if it exists
	if (theMesh->vertexNormals != NULL)
		{
			theMesh->vertexNormals[normalsCount++] = x;
			theMesh->vertexNormals[normalsCount++] = y;
			theMesh->vertexNormals[normalsCount++] = z;
		}
	else
		normalsCount = normalsCount + 3;
}

static void ReadOneFace(MeshPtr theMesh)
{
	int tokenType;
	bool triplets = false;

	// OBS! Unknown number! Can be one single vertex index or a triplet
	do
	{
		OBJGetToken(&tokenType);
	
		switch (tokenType)
		{
		case kReal: // Real should not happen
		case kInt:
			if (intValue[0] != 0)
			{
				hasPositionIndices = true;

				// Single index
				if (theMesh->coordIndex != NULL)
				{
					if (intValue[0] >= 0)
						theMesh->coordIndex[coordCount] = intValue[0]-1;
					else
						theMesh->coordIndex[coordCount] =
							vertCount / 3 + intValue[0];
				}
			}
		break;
	case tripletToken:
		// Triplet (out of which some may be missing)

		if (intValue[0] != 0)
		{
			hasPositionIndices = true;

			if (theMesh->coordIndex != NULL)
			{
				if (intValue[0] > 0)
					theMesh->coordIndex[coordCount] = intValue[0]-1;
				else
					theMesh->coordIndex[coordCount] =
						vertCount+intValue[0];
			}
		}
		if (intValue[1] != 0)
		{
			hasTexCoordIndices = true;

			if (theMesh->textureIndex != NULL)
			{
				if (intValue[1] > 0)
					theMesh->textureIndex[coordCount] = intValue[1]-1;
				else
					theMesh->textureIndex[coordCount] =
						texCount / 2 + intValue[1];
			}
			}
		if (intValue[2] != 0)
		{
			hasNormalIndices = true;

			if (theMesh->normalsIndex != NULL)
			{
				if (intValue[2] >= 0)
					theMesh->normalsIndex[coordCount] = intValue[2]-1;
				else
					theMesh->normalsIndex[coordCount] = 
						normalsCount / 3 + intValue[2];
				}
			}
			triplets = true;
			break;
		}

		coordCount++;
	}
	while ((tokenType != kEOF) && (tokenType != crlfToken) && !atLineEnd);

	// Terminate polygon with -1 (like VRML)
	if (theMesh->coordIndex != NULL)
	{
		theMesh->coordIndex[coordCount] = -1;
	}
	if (triplets)
	{
		if (theMesh->textureIndex != NULL)
		{
			theMesh->textureIndex[coordCount] = -1;
		}
		if (theMesh->normalsIndex != NULL)
		{
			theMesh->normalsIndex[coordCount] = -1;
		}
	}

	coordCount++;
}

static void ParseOBJ(MeshPtr theMesh)
{
	int tokenType;
	
	tokenType = 0;
	while (tokenType != kEOF)
	{
		OBJGetToken(&tokenType);
		switch (tokenType)
		{
		case vToken:
			ReadOneVertex(theMesh);
			break;
		case vnToken:
			ReadOneNormal(theMesh);
			break;
		case vtToken:
			ReadOneTexture(theMesh);
			break;
		case fToken:
			ReadOneFace(theMesh);
			break;
		case kReal:
			// Ignore
			break;
		case crlfToken:
			break;
		case kUnknown:
			SkipToCRLF();
			//while (tokenType != crlfToken && tokenType != kEOF)
			//	OBJGetToken(&tokenType);
			break;
		}
	}
}


static struct Mesh * LoadOBJ(const char *filename)
{
	Mesh *theMesh;
	
	theMesh = malloc(sizeof(Mesh));
	theMesh->coordIndex = NULL;
	theMesh->vertices = NULL;
	// ProcessMesh may deal with these
	theMesh->triangleCountList = NULL;
	theMesh->vertexNormals = NULL;
	theMesh->vertexToTriangleTable = NULL;
	theMesh->textureCoords = NULL;
	theMesh->textureIndex = NULL;
	theMesh->normalsIndex = NULL;

	hasPositionIndices = true;
	hasTexCoordIndices = false;
	hasNormalIndices = false;
	
	vertCount=0;
	texCount=0;
	normalsCount=0;
	coordCount=0;
	
	fp = fopen(filename, "rb"); // rw works everywhere except Windows?
	if (fp == NULL)
	{
		fprintf(stderr, "Unable to open file '%s'\n", filename);
		fflush(stderr);
		return NULL;
	}
	ParseOBJ(theMesh);
	fclose(fp);

	// Allocate arrays!
	if (vertCount > 0)
		theMesh->vertices = malloc(sizeof(GLfloat) * vertCount);
	if (texCount > 0)
		theMesh->textureCoords = malloc(sizeof(GLfloat) * texCount);
	if (normalsCount > 0)
		theMesh->vertexNormals = malloc(sizeof(GLfloat) * normalsCount);
	if (hasPositionIndices)
		theMesh->coordIndex = malloc(sizeof(int) * coordCount);
	if (hasNormalIndices)
		theMesh->normalsIndex = malloc(sizeof(int) * coordCount);
	if (hasTexCoordIndices)
		theMesh->textureIndex = malloc(sizeof(int) * coordCount);
	
	// Zero again
	vertCount=0;
	texCount=0;
	normalsCount=0;
	coordCount=0;

	fp = fopen(filename, "rb");
	if (fp == NULL) return NULL;
	ParseOBJ(theMesh);
	fclose(fp);
	
	theMesh->vertexCount = vertCount/3;
	theMesh->coordCount = coordCount;
	
	// Counters for tex and normals, texCount and normalsCount
	theMesh->texCount = texCount/2;
	theMesh->normalsCount = normalsCount/3; // Should be the same as vertexCount!
	// This assumption could make handling of some models break!
	
	return theMesh;
}

void DecomposeToTriangles(struct Mesh *theMesh)
{
	int i, vertexCount, triangleCount;
	int *newCoords, *newNormalsIndex, *newTextureIndex;
	int newIndex = 0; // Index in newCoords
	int first = 0;

	// 1. Bygg om hela modellen till trianglar
	// 1.1 Calculate how big the list will become
	
	vertexCount = 0; // Number of vertices in current polygon
	triangleCount = 0; // Resulting number of triangles
	for (i = 0; i < theMesh->coordCount; i++)
	{
		if (theMesh->coordIndex[i] == -1)
		{
		if (vertexCount > 2) triangleCount += vertexCount - 2;
			vertexCount = 0;
		}
		else
		{
			vertexCount = vertexCount + 1;
		}
	}
	
	fprintf(stderr, "Found %d triangles\n", triangleCount);
	
	newCoords = malloc(sizeof(int) * triangleCount * 3);
	if (theMesh->normalsIndex != NULL)
		newNormalsIndex = malloc(sizeof(int) * triangleCount * 3);
	if (theMesh->textureIndex != NULL)
		newTextureIndex = malloc(sizeof(int) * triangleCount * 3);
	
	// 1.2 Loop through old list and write the new one
	// Almost same loop but now it has space to write the result
	vertexCount = 0;
	for (i = 0; i < theMesh->coordCount; i++)
	{
		if (theMesh->coordIndex[i] == -1)
		{
			first = i + 1;
			vertexCount = 0;
		}
		else
		{
			vertexCount = vertexCount + 1;

			if (vertexCount > 2)
			{
				newCoords[newIndex++] = theMesh->coordIndex[first];
				newCoords[newIndex++] = theMesh->coordIndex[i-1];
				newCoords[newIndex++] = theMesh->coordIndex[i];
				
				if (theMesh->normalsIndex != NULL)
				{
					newNormalsIndex[newIndex-3] = theMesh->normalsIndex[first];
					newNormalsIndex[newIndex-2] = theMesh->normalsIndex[i-1];
					newNormalsIndex[newIndex-1] = theMesh->normalsIndex[i];
				}
				
				// Dito for textures
				if (theMesh->textureIndex != NULL)
				{
					newTextureIndex[newIndex-3] = theMesh->textureIndex[first];
					newTextureIndex[newIndex-2] = theMesh->textureIndex[i-1];
					newTextureIndex[newIndex-1] = theMesh->textureIndex[i];
				}
			
			}
		}
	}
	
	free(theMesh->coordIndex);
	theMesh->coordIndex = newCoords;
	theMesh->coordCount = triangleCount * 3;
	if (theMesh->normalsIndex != NULL)
	{
		free(theMesh->normalsIndex);
		theMesh->normalsIndex = newNormalsIndex;
	}
	if (theMesh->textureIndex != NULL)
	{
		free(theMesh->textureIndex);
		theMesh->textureIndex = newTextureIndex;
	}
} // DecomposeToTriangles


static void generateNormals(Mesh* mesh)
{
	// If model has vertices but no vertexnormals, generate normals
	if (mesh->vertices && !mesh->vertexNormals)
	{
		int face;
		int normalIndex;

		mesh->vertexNormals = malloc(3 * sizeof(GLfloat) * mesh->vertexCount);
		memset(mesh->vertexNormals, 0, 3 * sizeof(GLfloat) * mesh->vertexCount);

		mesh->normalsCount = mesh->vertexCount;

		mesh->normalsIndex = malloc(sizeof(GLuint) * mesh->coordCount);
		memcpy(mesh->normalsIndex, mesh->coordIndex,
			sizeof(GLuint) * mesh->coordCount);

		for (face = 0; face * 3 < mesh->coordCount; face++)
		{
			int i0 = mesh->coordIndex[face * 3 + 0];
			int i1 = mesh->coordIndex[face * 3 + 1];
			int i2 = mesh->coordIndex[face * 3 + 2];
			
			GLfloat* vertex0 = &mesh->vertices[i0 * 3];
			GLfloat* vertex1 = &mesh->vertices[i1 * 3];
			GLfloat* vertex2 = &mesh->vertices[i2 * 3];

			float v0x = vertex1[0] - vertex0[0];
			float v0y = vertex1[1] - vertex0[1];
			float v0z = vertex1[2] - vertex0[2];

			float v1x = vertex2[0] - vertex0[0];
			float v1y = vertex2[1] - vertex0[1];
			float v1z = vertex2[2] - vertex0[2];

			float v2x = vertex2[0] - vertex1[0];
			float v2y = vertex2[1] - vertex1[1];
			float v2z = vertex2[2] - vertex1[2];

			float sqrLen0 = v0x * v0x + v0y * v0y + v0z * v0z;
			float sqrLen1 = v1x * v1x + v1y * v1y + v1z * v1z;
			float sqrLen2 = v2x * v2x + v2y * v2y + v2z * v2z;

			float len0 = (sqrLen0 >= 1e-6) ? sqrt(sqrLen0) : 1e-3;
			float len1 = (sqrLen1 >= 1e-6) ? sqrt(sqrLen1) : 1e-3;
			float len2 = (sqrLen2 >= 1e-6) ? sqrt(sqrLen2) : 1e-3;

			float influence0 = (v0x * v1x + v0y * v1y + v0z * v1z) / (len0 * len1);
			float influence1 = -(v0x * v2x + v0y * v2y + v0z * v2z) / (len0 * len2);
			float influence2 = (v1x * v2x + v1y * v2y + v1z * v2z) / (len1 * len2);

			float angle0 = (influence0 >= 1.f) ? 0 : 
				(influence0 <= -1.f) ? PI : acos(influence0);
			float angle1 = (influence1 >= 1.f) ? 0 : 
				(influence1 <= -1.f) ? PI : acos(influence1);
			float angle2 = (influence2 >= 1.f) ? 0 : 
				(influence2 <= -1.f) ? PI : acos(influence2);

			float normalX = v1z * v0y - v1y * v0z;
			float normalY = v1x * v0z - v1z * v0x;
			float normalZ = v1y * v0x - v1x * v0y;

			GLfloat* normal0 = &mesh->vertexNormals[i0 * 3];
			GLfloat* normal1 = &mesh->vertexNormals[i1 * 3];
			GLfloat* normal2 = &mesh->vertexNormals[i2 * 3];

			normal0[0] += normalX * angle0;
			normal0[1] += normalY * angle0;
			normal0[2] += normalZ * angle0;

			normal1[0] += normalX * angle1;
			normal1[1] += normalY * angle1;
			normal1[2] += normalZ * angle1;

			normal2[0] += normalX * angle2;
			normal2[1] += normalY * angle2;
			normal2[2] += normalZ * angle2;
		}

		for (normalIndex = 0; normalIndex < mesh->normalsCount; normalIndex++)
		{
			GLfloat* normal = &mesh->vertexNormals[normalIndex * 3];
			float length = sqrt(normal[0] * normal[0] + normal[1] * normal[1]
							+ normal[2] * normal[2]);
			float reciprocalLength = 1.f;

			if (length > 0.01f)
				reciprocalLength = 1.f / length;

			normal[0] *= reciprocalLength;
			normal[1] *= reciprocalLength;
			normal[2] *= reciprocalLength;
		}
	}
}


static Model* generateModel(Mesh* mesh)
{
	// Convert from Mesh format (multiple index lists) to Model format
	// (one index list) by generating a new set of vertices/indices
	// and where new vertices have been created whenever necessary

	typedef struct
	{
		int positionIndex;
		int normalIndex;
		int texCoordIndex;
		int newIndex;
	} IndexTriplet;

	int hashGap = 6;

	int indexHashMapSize = (mesh->vertexCount * hashGap + mesh->coordCount);

	IndexTriplet* indexHashMap = malloc(sizeof(IndexTriplet)
							* indexHashMapSize);

	int numNewVertices = 0;
	int index;

	int maxValue = 0;
		
	Model* model = malloc(sizeof(Model));
	memset(model, 0, sizeof(Model));

	model->indexArray = malloc(sizeof(GLuint) * mesh->coordCount);
	model->numIndices = mesh->coordCount;

	memset(indexHashMap, 0xff, sizeof(IndexTriplet) * indexHashMapSize);

	for (index = 0; index < mesh->coordCount; index++)
	{
		IndexTriplet currentVertex = { -1, -1, -1, -1 };
		int insertPos = 0;
		if (mesh->coordIndex)
			currentVertex.positionIndex = mesh->coordIndex[index];
		if (mesh->normalsIndex)
			currentVertex.normalIndex = mesh->normalsIndex[index];
		if (mesh->textureIndex)
			currentVertex.texCoordIndex = mesh->textureIndex[index];

		if (maxValue < currentVertex.texCoordIndex)
			maxValue = currentVertex.texCoordIndex;
 
		if (currentVertex.positionIndex >= 0)
			insertPos = currentVertex.positionIndex * hashGap;

		while (1)
		{
			if (indexHashMap[insertPos].newIndex == -1)
				{
					currentVertex.newIndex = numNewVertices++;
					indexHashMap[insertPos] = currentVertex;
					break;
				}
			else if (indexHashMap[insertPos].positionIndex
				 == currentVertex.positionIndex
				 && indexHashMap[insertPos].normalIndex
				 == currentVertex.normalIndex
				 && indexHashMap[insertPos].texCoordIndex
				 == currentVertex.texCoordIndex)
				{
					currentVertex.newIndex = indexHashMap[insertPos].newIndex;
					break;
				}
			else
				insertPos++;
		} 

		model->indexArray[index] = currentVertex.newIndex;
	}

	if (mesh->vertices)
		model->vertexArray = malloc(sizeof(GLfloat) * 3 * numNewVertices);
	if (mesh->vertexNormals)
		model->normalArray = malloc(sizeof(GLfloat) * 3 * numNewVertices);
	if (mesh->textureCoords)
		model->texCoordArray = malloc(sizeof(GLfloat) * 2 * numNewVertices);
	
	model->numVertices = numNewVertices;

	for (index = 0; index < indexHashMapSize; index++)
	{
		if (indexHashMap[index].newIndex != -1)
		{
			if (mesh->vertices)
				memcpy(&model->vertexArray[3 * indexHashMap[index].newIndex],
					&mesh->vertices[3 * indexHashMap[index].positionIndex],
					3 * sizeof(GLfloat));

			if (mesh->vertexNormals)
				memcpy(&model->normalArray[3 * indexHashMap[index].newIndex],
					&mesh->vertexNormals[3 * indexHashMap[index].normalIndex],
					3 * sizeof(GLfloat));

			if (mesh->textureCoords)
			{
				model->texCoordArray[2 * indexHashMap[index].newIndex + 0]
					= mesh->textureCoords[2 * indexHashMap[index].texCoordIndex + 0];
				model->texCoordArray[2 * indexHashMap[index].newIndex + 1]
					= 1 - mesh->textureCoords[2 * indexHashMap[index].texCoordIndex + 1];
			}
		}
	}

	free(indexHashMap);

	return model;
}


Model* LoadModel(char* name)
{
	Model* model = 0;
	Mesh* mesh = LoadOBJ(name);

	if (!mesh)
		return 0;

	DecomposeToTriangles(mesh);

	generateNormals(mesh);

	model = generateModel(mesh);

	return model;
}


void CenterModel(Model *m)
{
	int i;
	float maxx = -1e10, maxy = -1e10, maxz = -1e10, minx = 1e10, miny = 1e10, minz = 1e10;
	
	for (i = 0; i < m->numVertices; i++)
	{
		if (m->vertexArray[3 * i] < minx) minx = m->vertexArray[3 * i];
		if (m->vertexArray[3 * i] > maxx) maxx = m->vertexArray[3 * i];
		if (m->vertexArray[3 * i+1] < miny) miny = m->vertexArray[3 * i+1];
		if (m->vertexArray[3 * i+1] > maxy) maxy = m->vertexArray[3 * i+1];
		if (m->vertexArray[3 * i+2] < minz) minz = m->vertexArray[3 * i+2];
		if (m->vertexArray[3 * i+2] > maxz) maxz = m->vertexArray[3 * i+2];
	}
	
	fprintf(stderr, "maxx %f minx %f \n", maxx, minx);
	fprintf(stderr, "maxy %f miny %f \n", maxy, miny);
	fprintf(stderr, "maxz %f minz %f \n", maxz, minz);

	for (i = 0; i < m->numVertices; i++)
	{
		m->vertexArray[3 * i] -= (maxx + minx)/2.0;
		m->vertexArray[3 * i+1] -= (maxy + miny)/2.0;
		m->vertexArray[3 * i+2] -= (maxz + minz)/2.0;
	}
}

void ScaleModel(Model *m, float sx, float sy, float sz)
{
	long i;
	for (i = 0; i < m->numVertices; i++)
	{
		m->vertexArray[3 * i] *= sx;
		m->vertexArray[3 * i+1] *= sy;
		m->vertexArray[3 * i+2] *= sz;
	}
}

// NEW for lab 2 2012
// Modified 2013, to do decent error reporting
// This code makes a lot of calls for rebinding variables just in case,
// and to get attribute locations. This is clearly not optimal, but the
// goal is stability.

void DrawModel(Model *m, GLuint program, char* vertexVariableName, char* normalVariableName, char* texCoordVariableName)
{
	if (m != NULL)
	{
		GLint loc;
		
		glBindVertexArray(m->vao);	// Select VAO

		glBindBuffer(GL_ARRAY_BUFFER, m->vb);
		loc = glGetAttribLocation(program, vertexVariableName);
		if (loc >= 0)
		{
			glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0); 
			glEnableVertexAttribArray(loc);
		}
		else
			fprintf(stderr, "DrawModel warning: '%s' not found in shader!\n", vertexVariableName);
		
		if (normalVariableName!=NULL)
		{
			loc = glGetAttribLocation(program, normalVariableName);
			if (loc >= 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, m->nb);
				glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(loc);
			}
			else
				fprintf(stderr, "DrawModel warning: '%s' not found in shader!\n", normalVariableName);
		}
	
		// VBO for texture coordinate data NEW for 5b
		if ((m->texCoordArray != NULL)&&(texCoordVariableName != NULL))
		{
			loc = glGetAttribLocation(program, texCoordVariableName);
			if (loc >= 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, m->tb);
				glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(loc);
			}
			else
				fprintf(stderr, "DrawModel warning: '%s' not found in shader!\n", texCoordVariableName);
		}

		glDrawElements(GL_TRIANGLES, m->numIndices, GL_UNSIGNED_INT, 0L);
	}
}

void DrawWireframeModel(Model *m, GLuint program, char* vertexVariableName, char* normalVariableName, char* texCoordVariableName)
{
	if (m != NULL)
	{
		GLint loc;
		
		glBindVertexArray(m->vao);	// Select VAO

		glBindBuffer(GL_ARRAY_BUFFER, m->vb);
		loc = glGetAttribLocation(program, vertexVariableName);
		if (loc >= 0)
		{
			glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0); 
			glEnableVertexAttribArray(loc);
		}
		else
			fprintf(stderr, "DrawWireframeModel warning: '%s' not found in shader!\n", vertexVariableName);
		
		if (normalVariableName!=NULL)
		{
			loc = glGetAttribLocation(program, normalVariableName);
			if (loc >= 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, m->nb);
				glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(loc);
			}
			else
				fprintf(stderr, "DrawWireframeModel warning: '%s' not found in shader!\n", normalVariableName);
		}
	
		// VBO for texture coordinate data NEW for 5b
		if ((m->texCoordArray != NULL)&&(texCoordVariableName != NULL))
		{
			loc = glGetAttribLocation(program, texCoordVariableName);
			if (loc >= 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, m->tb);
				glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(loc);
			}
			else
				fprintf(stderr, "DrawWireframeModel warning: '%s' not found in shader!\n", texCoordVariableName);
		}
		glDrawElements(GL_LINE_STRIP, m->numIndices, GL_UNSIGNED_INT, 0L);
	}
}
	
// BuildModelVAO2

// Called from LoadModelPlus and LoadDataToModel
// VAO and VBOs must already exist!
// Useful by its own when the model changes on CPU
void ReloadModelData(Model *m)
{
	glBindVertexArray(m->vao);
	
	// VBO for vertex data
	glBindBuffer(GL_ARRAY_BUFFER, m->vb);
	glBufferData(GL_ARRAY_BUFFER, m->numVertices*3*sizeof(GLfloat), m->vertexArray, GL_STATIC_DRAW);
	//glVertexAttribPointer(glGetAttribLocation(program, vertexVariableName), 3, GL_FLOAT, GL_FALSE, 0, 0); 
	//glEnableVertexAttribArray(glGetAttribLocation(program, vertexVariableName));
	
	// VBO for normal data
	glBindBuffer(GL_ARRAY_BUFFER, m->nb);
	glBufferData(GL_ARRAY_BUFFER, m->numVertices*3*sizeof(GLfloat), m->normalArray, GL_STATIC_DRAW);
	//glVertexAttribPointer(glGetAttribLocation(program, normalVariableName), 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(glGetAttribLocation(program, normalVariableName));
	
	// VBO for texture coordinate data NEW for 5b
	if (m->texCoordArray != NULL)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m->tb);
		glBufferData(GL_ARRAY_BUFFER, m->numVertices*2*sizeof(GLfloat), m->texCoordArray, GL_STATIC_DRAW);
		//glVertexAttribPointer(glGetAttribLocation(program, texCoordVariableName), 2, GL_FLOAT, GL_FALSE, 0, 0);
		//glEnableVertexAttribArray(glGetAttribLocation(program, texCoordVariableName));
	}
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->numIndices*sizeof(GLuint), m->indexArray, GL_STATIC_DRAW);
}

Model* LoadModelPlus(char* name/*,
			GLuint program,
			char* vertexVariableName,
			char* normalVariableName,
			char* texCoordVariableName*/)
{
	Model *m;
	
	m = LoadModel(name);
	
	glGenVertexArrays(1, &m->vao);
	glGenBuffers(1, &m->vb);
	glGenBuffers(1, &m->ib);
	glGenBuffers(1, &m->nb);
	if (m->texCoordArray != NULL)
		glGenBuffers(1, &m->tb);
		
	ReloadModelData(m);
	
	return m;
}

// Loader for inline data to Model (almost same as LoadModelPlus)
Model* LoadDataToModel(
			GLfloat *vertices,
			GLfloat *normals,
			GLfloat *texCoords,
			GLfloat *colors,
			GLuint *indices,
			int numVert,
			int numInd)
{
	Model* m = malloc(sizeof(Model));
	memset(m, 0, sizeof(Model));
	
	m->vertexArray = vertices;
	m->texCoordArray = texCoords;
	m->normalArray = normals;
	m->indexArray = indices;
	m->numVertices = numVert;
	m->numIndices = numInd;
	
	glGenVertexArrays(1, &m->vao);
	glGenBuffers(1, &m->vb);
	glGenBuffers(1, &m->ib);
	glGenBuffers(1, &m->nb);
	if (m->texCoordArray != NULL)
		glGenBuffers(1, &m->tb);

	ReloadModelData(m);
	
	return m;
}
