/// @file Utilities.h
/// This file contains useful functions that doesn't quite fit anywhere else.

#ifndef UTILITIES_H
#define UTILITIES_H
/*
#ifdef __cplusplus
extern "C" {
#endif
*/

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

/// @brief Converts string to floats
///
/// Takes the pointer to the beginning of a string and reads a float, ignoring starting whitespace. 
/// Handles sign and up to 9 decimals. Use strEnd to get a pointer to where the it stopped reading the string.
/// Most of this code comes from StackOverflow
/// @return A float value read from the string.
float myStrtof(char* strStart, char** strEnd);


float sumArray(int N, float** v);
void print(int N, float** v);
void zeroArray(const int N, float** x);
void oneArray(const int N, float** x);

/*
#ifdef __cplusplus
}
#endif
*/


#endif // UTILITIES_H
