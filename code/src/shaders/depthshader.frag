/// @file depthshader.frag
/// @brief Fragment shader for depth map rendering.

#version 150

// ===== Uniforms =====

uniform sampler2D height_texUnit;	///< Terrain geometry (normal and height) texture.
uniform vec3 size;					///< Data size.
uniform float maxDepth;				///< Max depth variable (for tweaking depth grayscale saturation).


// ===== In/Out params =====

in vec2 out_TexCoord;				///< Fragment texture coordinate.
in vec3 out_ObjPos;					///< Fragment position.

out vec4 out_Color;					///< Fragment (out) pixel value.


// ===== Variables needed =====

float depth;						///< Water depth at fragment.
vec4 terrainDataUnderSurface;		///< Terrain geometry under fragment.


void main(void)
{
	// Texture lookup at fragment.
	terrainDataUnderSurface = texture(height_texUnit, out_TexCoord);

	// Depth at fragment.
	depth = out_ObjPos.y - size.y * terrainDataUnderSurface.a;
	
	if (depth < 0 || out_ObjPos.z < 1 || out_ObjPos.x < 1 )
	{
		discard;
	}
	
	out_Color = vec4(vec3(1.0, 1.0, 1.0) * (1 - depth / maxDepth), 1.0);
}