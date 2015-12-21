/// @file watershader.frag
/// @brief Fragment shader for water rendering.

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
uniform float time;					///< Time variable.
uniform float transparency;			///< Transparency of the water.
uniform sampler2D terr_texUnit;		///< Terrain (color) texture.
uniform sampler2D height_texUnit;	///< Terrain geometry (normal and height) texture.
uniform samplerCube sky_texUnit;	///< Skybox texture.


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
vec3 Normal;						///< (Modified) fragment normal.
vec3 re;							///< Reflected eye vector.
vec3 right;							///< Vector to the right (camera position dependent).

// Lighting (the Phong model with extras).
float kamb;							///< Ambient light coefficient.
float kdiff;						///< Diffuse light coefficient.
float ktransr;						///< Transparent light (red channel) coefficient.
float ktransg;						///< Transparent light (green channel) coefficient.
float ktransb;						///< Transparent light (blue channel) coefficient.
float krefl;						///< Reflected light coefficient.
float kblue;						///< "Blueness" coefficient.

vec3 ambLight;						///< Ambient light color.
vec3 diffLight;						///< Diffuse light color.
vec3 specLight;						///< Specular light color.
vec3 reflLight;						///< Reflected light (skybox) color.
vec3 surfaceLight;					///< Total surface light color.
vec3 bottomLight;					///< Total bottom light color.

// Snell's law angles.
float theta1;						///< Incident light angle (from camera to bottom).
float theta2;						///< Refracted light angle (from camera to bottom).

// Underwater triangulation components.
float depth;						///< Water depth at fragment.
vec3 displacementDirection;			///< Refraction direction in the xz-plane.
float bottomDisplacement1;			///< Distance to displace along the bottom for the first approximation.
vec3 displacement1;					///< Displacement vector (first approximation).
float depthAtDis1;					///< Depth at first displacement approximation.
float h;							///< Height at first approximation.
float alpha;						///< Angle used for refraction approximation.
float bottomDisplacement2;			///< Better displacement approximation distance.
vec3 displacement2;					///< Better displacement approximation vector.
float depthAtDis2;					///< Depth at better approximation.
float wdist;						///< Distance from surface to better approximation.

vec3 bottomPos;						///< Position of seen bottom.
vec3 bottomNormal;					///< Normal of seen bottom.

// Texture lookups.
vec4 terrainDataUnderSurface;		///< Terrain geometry under fragment.
vec4 terrainDataAtDis1;				///< Terrain geometry at first approximation.
vec4 terrainDataAtBottom;			///< Terrain geometry at better approximation.
vec4 texDataAtBottom;				///< Terrain color at bottom.
vec3 skyrefl;						///< Skybox color.

// Constants
const float waterRefInd = 1.34451;	///< Refraction index of water.
const float airRefInd = 1.0;		///< Refraction index of air.
const vec3 up = vec3(0.0, 1.0, 0.0);///< Up vector (world coordinates).


void main(void)
{
	// eye vector is calculated.
	eye = normalize(camPos - out_ObjPos);
		
	// Modelling surface waves.
	float rho1 = sqrt(pow(out_ObjPos.x, 2) + pow(out_ObjPos.z, 2));
	float rho2 = sqrt(pow((size.x - float(out_ObjPos.x)), 2) + pow(out_ObjPos.z, 2));
	float xwave = sin(0.1 * time + out_ObjPos.x) + sin(-0.01 * time + out_ObjPos.x);
	float ywave = sin(0.2 * time + out_ObjPos.z) + sin(-0.3 * time + out_ObjPos.z);
	float rhowave1 = sin(0.05 * time + 0.5 + rho1) + 0.2 * sin(-0.01 * time + rho1);
	float rhowave2 = sin(-0.05 * time + 0.5 + rho2/10.0f) + 0.2 * sin(-0.01 * time + rho2 / 10.0f);
 	float rand = 1;
	rand = fract(sin(dot(vec2(out_ObjPos.x + time, out_ObjPos.z), vec2(12.9898, 78.233))) * 43758.5453);

	// Perturbing surface normals.
	Normal = vec3(0.1 * (0.1 * xwave + 0.1 * rhowave1 + rhowave2), out_Normal.y, 0.1 * 0.1 * ywave);
	Normal = normalize(0.5 * out_Normal + 0.5 * Normal);

	// Incident and reflected light is calculated for the light source.
	s = normalize(lights[0].pos - (1 - lights[0].isDir) * out_ObjPos);
	r = normalize(2 * Normal * dot(s, Normal) - s);

	right = cross(Normal, eye);
	// Snell's law
	// Since asin is not that cheap, approximations could be made here.
	theta1 = asin(length(right));
	theta2 = asin(airRefInd * sin(theta1) / waterRefInd);

	// Texture lookup at fragment.
	terrainDataUnderSurface = texture(height_texUnit, out_TexCoord);

	// Depth at fragment.
	depth = out_ObjPos.y - size.y * terrainDataUnderSurface.a;
	
	if (depth < 0 || out_ObjPos.z < 1 || out_ObjPos.x < 1 )
	{
		discard;
	}

	// To find the fragment at the bottom corresponding to what would be seen after refraction
	// at the water surface, a crude displacement approximation in the xz-plane from the surface
	// fragment to the approximated fragment at the bottom is calculated.
	vec3 s2 = airRefInd / waterRefInd * cross(Normal, cross(Normal, eye)) - Normal * sqrt(1 - pow(airRefInd / waterRefInd, 2) * dot(cross(Normal, eye), cross(Normal, eye)));
	displacementDirection = normalize(s2 - dot(s2, -up) * -up);
	bottomDisplacement1 = tan(theta2) * depth;
	displacement1 = bottomDisplacement1 * displacementDirection;

	// Texture lookup at approximation.
	terrainDataAtDis1 = texture(height_texUnit, out_TexCoord + vec2(displacement1.x / size.x, displacement1.z / size.z));

	// "Depth" at approximation.
	depthAtDis1 = out_ObjPos.y - size.y * terrainDataAtDis1.a;
	// Height at approximation (y distance from fragment bottom to approximation bottom).
	h = depth - depthAtDis1;
	// To minimize visual errors caused by irregular terrain, a better approximation is made.
	alpha = abs(atan(h / bottomDisplacement1));
	bottomDisplacement2 = cos(alpha) * depth * sin(theta2) / cos(alpha - theta2);
	displacement2 = bottomDisplacement2 * displacementDirection;

	// Texture lookups at better approximation.
	terrainDataAtBottom = texture(height_texUnit, out_TexCoord + vec2(displacement2.x / size.x, displacement2.z / size.z));
	texDataAtBottom = texture(terr_texUnit, out_TexCoord + vec2(displacement2.x / size.x, displacement2.z / size.z));
	// "Depth" at better approximation.
	depthAtDis2 = out_ObjPos.y - size.y * terrainDataAtBottom.a;

	// Coordinates and normal of seen position of bottom.
	bottomPos = out_ObjPos + displacement2 - vec3(0, depthAtDis2, 0);
	bottomNormal = normalize(terrainDataAtBottom.rgb);

	// Distance from surface to seen position of bottom.
	wdist = length(bottomPos - out_ObjPos);
	
	// Skybox reflection.
	// Reflected eye vector.
	re = 2 * dot(eye, Normal) * Normal - eye;
	skyrefl = texture(sky_texUnit, re).rgb;
	// Light components, water surface.
	kamb = 0.1;
	krefl = clamp((1 - eye.y), 0.2, 0.7);
	// Ambient light.
	ambLight = kamb * vec3(1.0, 1.0, 1.0);
	reflLight = vec3(0.0, 0.0, 0.0);
	// Reflected light.
	reflLight += krefl * skyrefl;
	
	surfaceLight = vec3(0.0, 0.0, 0.0);
	// The light components are added to the total surface light.
	surfaceLight += ambLight;
	surfaceLight += reflLight;

	ktransr = clamp((1 - reflLight.r) * pow((1 + wdist), -1.8 * transparency), 0.0, 0.5);
	ktransg = clamp((1 - reflLight.g) * pow((1 + wdist), -1.7 * transparency), 0.0, 0.5);
	ktransb = clamp((1 - reflLight.b) * pow((1 + wdist), -1.5 * transparency), 0.0, 0.5);
	kblue = 1 - eye.y;

	// Calculating bottom light.
	// Phong shading for the bottom.
	s = normalize(lights[1].pos - (1 - lights[1].isDir) * bottomPos);
	r = normalize(2 * bottomNormal * dot(s, bottomNormal) - s);

	// eye vector is calculated (note: from bottom to surface, not to camera).
	eye = normalize(out_ObjPos - bottomPos);

	// Light according to the Phong model.
	kamb = 0.1;
	kdiff = 0.5;
	krefl = 0.5;
	ambLight = kamb * vec3(1.0, 1.0, 1.0);
	diffLight = vec3(0.0, 0.0, 0.0);
	specLight = vec3(0.0, 0.0, 0.0);
	// Diffuse light.
	diffLight += kdiff * lights[1].color * max(0.0, dot(s, bottomNormal));
	// Specular light.
	specLight += krefl * lights[1].color * max(0.0, pow(dot(r, eye), lights[1].specExp));

	bottomLight = vec3(0.0, 0.0, 0.0);
	// The light components are added to the total bottom light.
	bottomLight += ambLight;
	bottomLight += diffLight;
	bottomLight += specLight;
	
	bottomLight *= texDataAtBottom.rgb;
	//Color dependant attenuation.
	bottomLight = vec3(bottomLight.r * ktransr, bottomLight.g * ktransg, bottomLight.b * ktransb);.
	
	out_Color = vec4(bottomLight + surfaceLight, 1.0);
}