/// @file Utilities.h
/// This file contains useful functions that doesn't quite fit anywhere else.

#ifndef UTILITIES_H
#define UTILITIES_H

#include "loadobj.h"

/// @brief Generates a square canvas for FBO drawing
///
/// Returns a model that can be useful for filtering and other tasks where a FBO is used.
/// @return A square flat model.
Model* generateCanvas();

/// @brief Generate a cube
///
/// Returns a model that can be used for a skycube for example
/// @return A cube of scale s
Model* generateCube(GLfloat s);

/// @brief Frees the memory used by the model
///
/// All pointers inside the model are freed and finally the model too.
/// @warning This deallocates each buffer object the model is not renderable after this has been called
/// @note The pointer passed as input is freed do no use it after alling this function.
/// @return A square flat model.
void releaseModel(Model* m);

/// @brief Converts a string of integers to an integer value
///	
/// Accepts a starting point value which will be added to at the end, i.e. the value will be 
/// multiplied by 10 for each new number read from the string.
/// @param strStart Pointer to where the string starts
/// @param strEnd Pointer to where it stopped reading
/// @param val Starting value to add to, will be multiplied by ten for each integer read.
/// @return The integers read from the string or added to the input.
uint64_t myStrtol(char* strStart, char** strEnd, uint64_t val);

/// @brief Converts string to floats
///
/// Takes the pointer to the beginning of a string and reads a float, ignoring starting whitespace. 
/// Handles sign and up to 9 decimals. Use strEnd to get a pointer to where the it stopped reading the string.
/// Most of this code comes from StackOverflow
/// @param strStart Pointer to where the string starts
/// @param strEnd Pointer to where it stopped reading
/// @return A float value read from the string.
float myStrtof(char* strStart, char** strEnd);

/// @brief Compiles a compute shader
///
/// Creates and compiles a compute shader using the source code in the specified path
/// @param location The path to the source for the compute shader
/// @return The program ID
GLuint compileComputeShader(const char* location);


#endif // UTILITIES_H
