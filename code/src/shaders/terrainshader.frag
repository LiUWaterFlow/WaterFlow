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

in vec3 out_Normal;
in vec2 out_TexCoord;// not currently used
in vec3 out_ObjPos;

out vec4 out_Color;

uniform vec3 camPos;	// Kamerapositionen.

uniform vec3 size;

uniform sampler2D terr_texUnit;
uniform sampler2D height_texUnit;	// Terrain normal and height texture.

vec3 r;
vec3 s;				// Incomming light.
vec3 eye;			// vector from object to camera.


// Phong-modellen:
float kamb;
float kdiff;
float kspec;
vec3 ambLight;		// Ambient.
vec3 diffLight;		// Diffuse.
vec3 specLight;		// Specular.
vec3 totalLight;	// total light.

//in float out_Col_i;
//in float out_Col_j;

void main(void)
{
	// Incomming and reflecting light is calculated for every lightsource
	s = normalize(lights[0].pos - (1 - lights[0].isDir) * out_ObjPos);
	r = normalize(2 * out_Normal * dot(normalize(s), normalize(out_Normal)) - s);

	// eye-vector is calculated.
	eye = normalize(camPos - out_ObjPos);

	// Light according to phong model
	kamb = 0.1;
	kdiff = 0.5;
	kspec = 0.5;
	ambLight = kamb * vec3(1.0, 1.0, 1.0);
	diffLight = vec3(0.0, 0.0, 0.0);
	specLight = vec3(0.0, 0.0, 0.0);
	// Diffuse-light calculated
	diffLight += kdiff * lights[0].color * max(0.0, dot(s, normalize(out_Normal)));
	// Specular light calculated
	specLight += kspec * lights[0].color * max(0.0, pow(dot(r, eye), lights[0].specExp));

	totalLight = vec3(0.0, 0.0, 0.0);
	// the different light components is added to total light
	totalLight += ambLight;
	totalLight += diffLight;
	totalLight += specLight;

	// Just to check that the terrain data texture is working
	vec3 terrainData = vec3(texture(terr_texUnit, out_TexCoord)) * totalLight;
	out_Color = vec4(terrainData, 1.0f);
}
