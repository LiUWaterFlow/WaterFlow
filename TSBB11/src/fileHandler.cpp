/// @file fileHandler.cpp
/// @brief Implementations of functions fileHandler.h

#include "fileHandler.h"

// ===== Constructor =====

fileHandler::fileHandler(std::string path)
{
	fPath = path;
}

// ===== Methods =====

void fileHandler::SaveData(float fArr[], int aSize, int dWidth, int dHeight)
{
	fArray = fArr;
	arrayLength = aSize;
	width = dWidth;
	height = dHeight;
	std::ofstream outFile;
	outFile.open(fPath, std::ios_base::binary);
	int32_t w32 = width;
	int32_t h32 = height;
	char const * w = reinterpret_cast<char const *>(&w32);
	char const * h = reinterpret_cast<char const *>(&h32);
	outFile.write(w, 4);
	outFile.write(h, 4);
	for (int n = 0; n < arrayLength; n++)
	{
		char const * p = reinterpret_cast<char const *>(&fArray[n]);
		outFile.write(p, 4);
	}
	outFile.close();
}

int fileHandler::LoadData()
{
	std::ifstream inFile;
	inFile.open(fPath, std::ios::ate);
	// Returns -1 if file cannot be read.
	if (!inFile.is_open())
	{
		return -1;
	}
	int bCount = ((int(inFile.tellg())) - 2 * sizeof(int32_t));
	arrayLength = bCount / 4;
	fArray = new float[arrayLength];

	inFile.close();
	inFile.open(fPath, std::ios_base::binary);

	char buf[4];
	int32_t whTemp;
	inFile.read(buf, 4);
	memcpy(&whTemp, &buf, sizeof(whTemp));
	width = whTemp;
	inFile.read(buf, 4);
	memcpy(&whTemp, &buf, sizeof(whTemp));
	height = whTemp;

	int n = 0;
	while (n < arrayLength)
	{
		inFile.read(buf, 4);
		memcpy(&fArray[n], &buf, 4);
		n++;
	}
	return 0;
}

// ===== Getters =====

float* fileHandler::GetArray()
{
	return fArray;
}

int fileHandler::GetArrayLength()
{
	return arrayLength;
}

int fileHandler::GetDataWidth()
{
	return width;
}
int fileHandler::GetDataHeight()
{
	return height;
}