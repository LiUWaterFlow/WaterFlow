/// @file Utilities.h
/// This file contains useful functions that doesn't quite fit anywhere else.

#ifndef UTILITIES_H
#define UTILITIES_H

#ifdef __cplusplus
extern "C" {
#endif


#include "loadobj.h"

/// @brief Generates a square canvas for FBO drawing
///
/// Returns a model that can be useful for filtering and other tasks where a FBO is used.
/// @return A square flat model.
Model* generateCanvas();

/// @brief Frees the memory used by the model
///
/// All pointers inside the model are freed and finally the model too.
/// @note The pointer passed as input is freed do no use it after alling this function.
/// @return A square flat model.
void releaseModel(Model* m);

#ifdef __cplusplus
}
#endif

#endif // UTILITIES_H