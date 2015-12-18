/// @file fileHandler.h
/// @brief Loads and stores float arrays.

#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <fstream>
#include <string.h>
#include <vector>
#include <cstdint>

/// @class fileHandler
/// @brief Used to store float arrays, and/or load them from files.
///
/// Use the fileHandler class to either store a generic float array, with some side information,
/// or to load side information and a float array from the specified path.
/// When attempting to load a file that does not exist, LoadData() returns -1, otherwise 0.
class FileHandler
{
private:
	float* fArray;				///< Pointer to the float array.
	int arrayLength;			///< Length of the float array.
	int width;					///< Width (side information).
	int height;					///< Height (side information).
public:
	/// @brief Constructor, takes file path as parameter.
	///
	/// path must either point to a file saved with this class,
	/// or towards a location where data should be saved.
	/// @param path path to data file.
	/// @see SaveData()
	/// @see LoadData()
	FileHandler(int dWidth, int dHeight);

	/// @brief Stores data, both as variables in the object and as a file
	///
	/// specified by fPath. Note that the file indicated by fPath will be
	/// created if it does not exist, and it will be overwritten if it does exist.
	/// @param fArr float array to store.
	/// @param aSize length of the float array.
	/// @param dWidth width (side information to store).
	/// @param dHeight height (side information to store).
	void SaveData(std::string path);

	/// @brief Loads data from a file specified by fPath into the object.
	///
	/// Note that this method simply interprets a file as if it were saved with SaveData().
	/// If the file specified by fPath is of another origin, the data loaded could be flagged
	/// as being correctly loaded, but in reality make little sense.
	/// @return 0 if data was correctly loaded, -1 if the file could not be read.
	/// @see SaveData()
	int LoadData(std::string path);

	/// @brief Getter for the stored float array.
	/// @return pointer to the float array.
	float** GetArray();

	/// @brief Getter for the stored float array length.
	/// @return length of the float array.
	int GetArrayLength();

	/// @brief Getter for the data width.
	/// @return width (side information).
	int GetDataWidth();

	/// @brief Getter for the data height.
	/// @return height (side information).
	int GetDataHeight();
};

#endif // FILEHANDLER_H
