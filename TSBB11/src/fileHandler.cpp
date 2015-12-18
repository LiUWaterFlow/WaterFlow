/// @file fileHandler.cpp
/// @brief Implementations of functions FileHandler.h

#include "fileHandler.h"
#include <iostream>

// ===== Constructor =====

FileHandler::FileHandler(int dWidth, int dHeight)
{
	fArray = nullptr;
	width = dWidth;
	height = dHeight;
	arrayLength = width * height;	
}

// ===== Methods =====

void FileHandler::SaveData(std::string path)
{
	std::ofstream outFile;//(path,std::fstream::trunc);
	outFile.open(path, std::fstream::binary | std::fstream::trunc | std::fstream::out);
	if(!outFile.is_open()){
		std::cerr << "File not opened for saving" << std::endl;
	}
	
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

int FileHandler::LoadData(std::string path)
{
	std::ifstream inFile;
	inFile.open(path, std::ios::ate);
	// Returns -1 if file cannot be read.
	if (!inFile.is_open())
	{
		return -1;
	}
	int bCount = ((int(inFile.tellg())) - 2 * sizeof(int32_t));
	arrayLength = bCount / 4;
	fArray = new float[arrayLength];

	inFile.close();
	inFile.open(path, std::ios_base::binary);

	char buf[4];
	int32_t whTemp;
	
	inFile.read(buf, 4);
	memcpy(&whTemp, &buf, sizeof(whTemp));
	if ( width != whTemp) {
		std::cerr << "Load data width doesn't match DEM data width" << std::endl;
		return -1;
	}
	
	width = whTemp;
	
	inFile.read(buf, 4);
	memcpy(&whTemp, &buf, sizeof(whTemp));
	
	if ( height != whTemp) {
		std::cerr << "Load data height doesn't match DEM data height" << std::endl;
		return -1;
	}
	
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

float** FileHandler::GetArray()
{
	return &fArray;
}

int FileHandler::GetArrayLength()
{
	return arrayLength;
}

int FileHandler::GetDataWidth()
{
	return width;
}
int FileHandler::GetDataHeight()
{
	return height;
}
