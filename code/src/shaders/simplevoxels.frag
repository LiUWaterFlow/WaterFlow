/// @file simplevoxels.frag
/// @brief Fragment shader to draw a simple representation of the voxel grid.

#version 150

out vec4 outColor; ///< The color to be set

void main()
{
	outColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);
}
