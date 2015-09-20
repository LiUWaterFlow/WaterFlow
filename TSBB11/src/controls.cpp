#include "../inc/controls.h"

void MouseSetCamera(float fi, float theta, vec3 camPos, vec3 *camLookAtPoint, vec3 camUp, vec3 *camForward, mat4 *viewMatrix)
{
	// Sets camForward, the direction (vector) the camera is facing.
	camForward->z = sinf(theta) * cosf(fi);
	camForward->x = sinf(theta) * sinf(fi);
	camForward->y = cosf(theta);

	// Translates this into camLookAtPoint, the point "just in front of the camera" when looking along camForward.
	*camLookAtPoint = camPos + *camForward;
	*viewMatrix = lookAtv(camPos, *camLookAtPoint, camUp);
}

void CheckKeys(vec3 *camPos, vec3 *camLookAtPoint, vec3 camUp, vec3 camForward, mat4 *viewMatrix)
{
	// Note: For now, keyIsDown only properly deals with upper case characters, i.e. 'W' instead of 'w', etc.
	float moveSpeed = 0.1;
	// 'w' moves the camera forwards.
	if (keyIsDown('W'))
	{
		*camPos += moveSpeed * camForward;
	}
	// 'a' moves the camera to the left.
	if (keyIsDown('A'))
	{
		*camPos -= moveSpeed * Normalize(CrossProduct(camForward, camUp));
	}
	// 'w' moves the camera backwards.
	if (keyIsDown('S'))
	{
		*camPos -= moveSpeed * camForward;
	}
	// 'd' moves the camera to the left.
	if (keyIsDown('D'))
	{
		*camPos += moveSpeed * Normalize(CrossProduct(camForward, camUp));
	}
	// 'a' moves the camera up.
	if (keyIsDown('E'))
	{
		*camPos += moveSpeed * camUp;
	}
	// 'c' moves the camera to the down.
	if (keyIsDown('C'))
	{
		*camPos -= moveSpeed * camUp;
	}

	// Updates the camera variables.
	*camLookAtPoint = *camPos + camForward;
	*viewMatrix = lookAtv(*camPos, *camLookAtPoint, camUp);
}