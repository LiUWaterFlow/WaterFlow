/// @file skyshader.vert
/// @brief Vertex shader for skybox rendering

#version 150

in vec3 in_Position; ///< Input vertex position

uniform mat4 VTPMatrix; ///< Transformation matrix from View to Projection space
uniform mat4 WTVMatrix; ///< Transformation matrix from World to View space

out vec3 Position; ///< Output vertex position


void main(void) {
	Position = in_Position;
	gl_Position = VTPMatrix * vec4(mat3(WTVMatrix) * in_Position, 1.0);
}
