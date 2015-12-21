/// @file terrainshader.frag
/// @brief Fragment shader for terrain rendering

#version 430

// ===== Uniform Buffers =====

/// @ struct LightParam
/// @ brief Struct that contains one light source
///
/// Load an array of light params as a uniform buffer, that makes it possible to
/// upload the lights only once.
struct LightParam {
	vec3 pos;		///< Position of the light
	float isDir;	///< Is the light directional or not
	vec3 color;		///< Color of the light
	float specExp;	///< Specular exponent for the light
};

layout(std140, binding = 0) uniform LightInfo {
	LightParam lights[2];
};


// ===== Uniforms =====

uniform vec3 camPos;				///< Camera position.
uniform vec3 size;					///< Data size.
uniform sampler2D terr_texUnit;		///< Terrain (color) texture.
uniform sampler2D height_texUnit;	///< Terrain geometry (normal and height) texture.


// ===== In/Out params =====

in vec3 out_Normal;					///< Fragment normal.
in vec2 out_TexCoord;				///< Fragment texture coordinate.
in vec3 out_ObjPos;					///< Fragment position.

out vec4 out_Color;					///< Fragment (out) pixel value.


// ===== Variables needed =====

// Light vectors
vec3 r;								///< Reflected light vector.
vec3 s;								///< Incident light vector.
vec3 eye;							///< Vector from the fragment to the camera.

// Lighting (the Phong model).
float kamb;							///< Ambient light coefficient.
float kdiff;						///< Diffuse light coefficient.
float kspec;						///< Specular light coefficient.

vec3 ambLight;						///< Ambient light color.
vec3 diffLight;						///< Diffuse light color.
vec3 specLight;						///< Specular light color.
vec3 totalLight;					///< Total light color.


void main(void)
{
	// Incident and reflected light is calculated for the light source.
	s = normalize(lights[0].pos - (1 - lights[0].isDir) * out_ObjPos);
	r = normalize(2 * out_Normal * dot(s, normalize(out_Normal)) - s);

	// eye vector is calculated.
	eye = normalize(camPos - out_ObjPos);

	// Light according to the Phong model.
	kamb = 0.1;
	kdiff = 0.5;
	kspec = 0.5;
	ambLight = kamb * vec3(1.0, 1.0, 1.0);
	diffLight = vec3(0.0, 0.0, 0.0);
	specLight = vec3(0.0, 0.0, 0.0);
	// Diffuse light.
	diffLight += kdiff * lights[0].color * max(0.0, dot(s, normalize(out_Normal)));
	// Specular light.
	specLight += kspec * lights[0].color * max(0.0, pow(dot(r, eye), lights[0].specExp));

	totalLight = vec3(0.0, 0.0, 0.0);
	// The different light components are added to the total light.
	totalLight += ambLight;
	totalLight += diffLight;
	totalLight += specLight;

	vec3 terrainData = vec3(texture(terr_texUnit, out_TexCoord)) * totalLight;
	out_Color = vec4(terrainData, 1.0f);
}