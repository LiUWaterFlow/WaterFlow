/// @file simplevoxels.vert
/// @brief Vertex shader to draw a simple representation of the voxel grid.

#version 150

in vec3 posValue;

uniform mat4 WTVMatrix;

void main()
{	
	gl_Position = WTVMatrix * vec4(posValue, 1.0f);
}
