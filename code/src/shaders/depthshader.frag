/// @file depthshader.frag
/// @brief Fragment shader for depth map rendering

#version 150

in vec2 out_TexCoord;
in vec3 out_ObjPos;

out vec4 out_Color;

uniform sampler2D height_texUnit;	// Terrain normal and height texture.

uniform vec3 size;			// Height scale.
uniform float maxDepth;

// Underwater triangulation components.
float depth;

// Texture lookups.
vec4 terrainDataUnderSurface;

void main(void)
{
	// Texture lookup at fragment.
	terrainDataUnderSurface = texture(height_texUnit, out_TexCoord);

	// Depth at fragment.
	depth = out_ObjPos.y - size.y * terrainDataUnderSurface.a;
	
	if (depth < 0 || out_ObjPos.z < 1 || out_ObjPos.x < 1 ){
		discard;
	}
	
	out_Color = vec4(vec3(1.0, 1.0, 1.0) * (1 - depth / maxDepth), 1.0);
}
