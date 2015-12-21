/// @file simplevoxels.vert
/// @brief Vertex shader to draw a simple representation of the voxel grid.

#version 150

in vec3 posValue; ///< Input point position

uniform mat4 WTVMatrix; ///< Transformation matrix from World to View space

void main()
{	
	gl_Position = WTVMatrix * vec4(posValue, 1.0f);
}
