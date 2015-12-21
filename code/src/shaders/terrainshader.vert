/// @file terrainshader.vert
/// @brief Vertex shader for model rendering
///
/// Used to render all height maps i.e. terrain, water and shallow water.

#version 430

// ===== Uniform Buffers =====

layout (std430,binding = 6) buffer velocity
{
	float v[];
	
};


// ===== Uniforms =====

uniform mat4 WTVMatrix;				///< World-to-view matrix.
uniform mat4 VTPMatrix;				///< View-to-projection matrix.


// ===== In/Out params =====

in vec3 in_Normal;					///< Vertex normal.
in vec2 in_TexCoord;				///< Vertex texture coordinates.
in vec3 in_Position;				///< Vertex position.

out vec3 out_Normal;				///< Fragment normal.
out vec2 out_TexCoord;				///< Fragment texture coordinates.
out vec3 out_ObjPos;				///< Fragment position.


void main(void)
{
	out_Normal = in_Normal;
	out_TexCoord = in_TexCoord;
	out_ObjPos = in_Position;

	gl_Position = VTPMatrix * WTVMatrix * vec4(in_Position, 1.0);
}