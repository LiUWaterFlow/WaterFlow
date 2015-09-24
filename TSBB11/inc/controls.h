#include "GL_utilities.h"
#include "VectorUtils3.h"

// Sets the camera variables after mouse movement.
void MouseSetCamera(float fi, float theta, vec3 camPos, vec3 *camLookAtPoint, vec3 camUp, vec3 *camForward, mat4 *viewMatrix);
// Checks if keys are being pressed.
void CheckKeys(vec3 *camPos, vec3 *camLookAtPoint, vec3 camUp, vec3 camForward, mat4 *viewMatrix);