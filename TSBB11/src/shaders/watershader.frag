#version 430

// ===== Uniform Buffers =====

struct LightParam {
	vec3 position;
	uint isDirectional;
	vec3 color;
	float specularExp;
};

struct DrawParam {
	float transparency;
	sampler2D terrainTexUnit;
	samplerCube skyTexUnit;
	uint padding3;
};

struct MapParam {
	vec2 size;
	sampler2D heightTexUnit;
	uint padding3;
};

layout(std430, binding = 0) uniform MapInfo {
	MapParam map;
};

layout(std430, binding = 1) uniform LightInfo {
	LightParam lights[2];
};

layout(std430, binding = 2) uniform DrawInfo {
	DrawParam draw;
};

// ===== Uniforms =====

uniform vec3 camPos;				// Camera position.

// ===== In/Out params =====

in vec3 out_Normal;
in vec2 out_TexCoord;
in vec3 out_ObjPos;

out vec4 out_Color;

// ===== Variables needed =====

// Light vectors
vec3 r;				// Reflected light.
vec3 s;				// Incident light.
vec3 eye;			// Vector from the fragment to the camera.
vec3 Normal;		// (Modified) fragment normal.
vec3 re;			// Reflected eye vector.
vec3 right;			// Vector to the right (camera position dependent).

// Lighting (the Phong model with extras).
float kamb;
float kdiff;
float ktrans;
float krefl;
float kblue;
vec3 ambLight;		// Ambient.
vec3 diffLight;		// Diffuse.
vec3 specLight;		// Specular.
vec3 reflLight;		// Reflected light (skybox).
vec3 surfaceLight;	// Total surface light.
vec3 surfaceColor;
vec3 bottomLight;	// Total bottom light.

// Snell's law angles.
float theta1;		// Incident light angle.
float theta2;		// Refracted light angle.

// Underwater triangulation components.
float depth;
vec3 displacementDirection;
float bottomDisplacement1;
vec3 displacement1;
float depthAtDis1;
float h;
float alpha;
float bottomDisplacement2;
vec3 displacement2;
float depthAtDis2;
float wdist;

vec3 bottomPos;
vec3 bottomNormal;

// Texture lookups.
vec4 terrainDataUnderSurface;
vec4 terrainDataAtDis1;
vec4 terrainDataAtBottom;
vec4 texDataAtBottom;
vec3 skyrefl;

// Constants
const float waterRefInd = 1.34451;
const float airRefInd = 1.0;
const vec3 up = vec3(0.0, 1.0, 0.0);

// ===== Replaced with the buffers =====
//uniform vec3 lightSourcePosAir;		// Light source position/direction, airborne light.
//uniform vec3 lightSourcePosWater;	// Light source position/direction, refracted light.
									// Currently this is precalculated on the CPU. This is not possible
									// when the water body comes from the actual simulation. This should
									// instead be calculated here.
//uniform int isDirectional;
//uniform float specularExponent;
//uniform vec3 lightSourceColor;
//uniform sampler2D terr_texUnit;		// Terrain texture.
//uniform sampler2D height_texUnit;	// Terrain normal and height texture.
//uniform samplerCube sky_sampler;	// Skybox texture.

//uniform int texCoordScaleH;			// Texture coordinate scale along x.
//uniform int texCoordScaleW;			// Texture coordinate scale along z.
//uniform float max_height;			// Height scale.
//uniform float time;
//uniform float transparency;

void main(void)
{
	// eye vector is calculated.
	eye = camPos - out_ObjPos;
	//float dist = length(eye);
	eye = normalize(eye);

	// Modelling surface waves.
	float rho1 = sqrt(pow(out_ObjPos.x, 2) + pow(out_ObjPos.z, 2));
	float rho2 = sqrt(pow((1000 - out_ObjPos.x), 2) + pow(out_ObjPos.z, 2));
	float xwave = sin(0.1 * time + out_ObjPos.x) + sin(-0.01 * time + out_ObjPos.x);
	float ywave = sin(0.2 * time + out_ObjPos.z) + sin(-0.3 * time + out_ObjPos.z);
	float rhowave1 = sin(0.5 * time + 0.5 + rho1) + 0.2 * sin(-0.1 * time + rho1);
	float rhowave2 = sin(-0.5 * time + 0.5 + rho2/10) + 0.2 * sin(-0.1 * time + rho2 / 10);
 	float rand = 1;
	rand = fract(sin(dot(vec2(out_ObjPos.x + time, out_ObjPos.z), vec2(12.9898, 78.233))) * 43758.5453);
	// Perturbing surface normals.
	Normal = vec3(0.1 * (0.1 * xwave + 0.1 * rhowave1 + rhowave2), out_Normal.y, 0.1 * 0.1 * ywave);
	//Normal += vec3(1 + 0.2 * rand, 1, 1);

	// Incident and reflected light is calculated for the light source.
	s = normalize(vec3(lightSourcePosAir.x, lightSourcePosAir.y, lightSourcePosAir.z) - (1 - isDirectional) * out_ObjPos);
	r = normalize(2 * Normal * dot(normalize(s), Normal) - s);

	right = cross(Normal, eye);
	// Snell's law
	// Since asin is not that cheap, approximations could be made here.
	theta1 = asin(length(right));
	theta2 = asin(airRefInd * sin(theta1) / waterRefInd);

	// Texture lookup at fragment.
	terrainDataUnderSurface = texture(height_texUnit, out_TexCoord);

	// Depth at fragment.
	depth = out_ObjPos.y - max_height * terrainDataUnderSurface.a;

	// Crude displacement approximation.
	displacementDirection = normalize(cross(up, right));
	bottomDisplacement1 = tan(theta2) * depth;
	displacement1 = bottomDisplacement1 * displacementDirection;

	// Texture lookup at approximation.
	// Eric: "but changing W and H looks better..."
	terrainDataAtDis1 = texture(height_texUnit, out_TexCoord + vec2(displacement1.x / texCoordScaleH, displacement1.z / texCoordScaleW));

	// To minimize negative depth values, a better approximation is made.
	// "Depth" at approximation.
	depthAtDis1 = out_ObjPos.y - max_height * terrainDataAtDis1.a;
	// Height at approximation (y distance from fragment bottom to approximation bottom).
	h = depth - depthAtDis1;
	// Better approximation
	alpha = abs(atan(h / bottomDisplacement1));
	bottomDisplacement2 = cos(alpha) * depth * sin(theta2) / cos(alpha - theta2);
	displacement2 = bottomDisplacement2 * displacementDirection;

	// Texture lookups at better approximation
	terrainDataAtBottom = texture(height_texUnit, out_TexCoord + vec2(displacement2.x / texCoordScaleH, displacement2.z / texCoordScaleW));
	texDataAtBottom = texture(terr_texUnit, out_TexCoord + vec2(displacement2.x / texCoordScaleH, displacement2.z / texCoordScaleW));
	// "Depth" at better approximation
	depthAtDis2 = out_ObjPos.y - max_height * terrainDataAtBottom.a;

	// Coordinates and normal of seen position of bottom.
	bottomPos = out_ObjPos + displacement2 - vec3(0, depthAtDis2, 0);
	bottomNormal = vec3(terrainDataAtBottom.x, terrainDataAtBottom.y, terrainDataAtBottom.z);

	// Distance from surface to seen position of bottom.
	wdist = length(bottomPos - out_ObjPos);
	
	// Skybox reflection.
	// Reflected eye vector.
	re = 2 * dot(eye, Normal) * Normal - eye;
	skyrefl = texture(sky_sampler, re).rgb;
	// Light components, water surface.
	kamb = 0.1;
	krefl = 0.4;
	// Ambient light.
	ambLight = kamb * vec3(1.0, 1.0, 1.0);
	reflLight = vec3(0.0, 0.0, 0.0);
	// Reflected light.
	reflLight += krefl * skyrefl;
	// Transmitted light coefficient.
	// --- Old code ---
	//float transexp = transparency;	// -----Not sure why this is needed.-----
	//ktrans = clamp(pow((1 + wdist), -transexp), 0, 0.3);
	//kblue = 1 - eye.y;
	//surfaceColor = (0.5 + kblue) * vec3(0.1, 0.2, 0.4);
	// ----------------
	// --- New code ---
	float maxDepthColor = 200 * transparency;
	ktrans = clamp(length(bottomPos - out_ObjPos), 0, maxDepthColor);
	ktrans = 1 - 1 / maxDepthColor * ktrans;
	surfaceColor = vec3(0.1, 0.2, 0.4);
	// ----------------

	surfaceLight = vec3(0.0, 0.0, 0.0);
	// The light components are added to the total surface light.
	surfaceLight += ambLight;
	//surfaceLight += surfaceColor;
	surfaceLight += reflLight;

	// Phong lighting for the bottom.
	s = normalize(vec3(lightSourcePosWater.x, lightSourcePosWater.y, lightSourcePosWater.z) - (1 - isDirectional) * bottomPos);
	r = normalize(2 * bottomNormal * dot(normalize(s), normalize(bottomNormal)) - s);

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
	diffLight += kdiff * lightSourceColor * max(0.0, dot(s, normalize(bottomNormal))) * texDataAtBottom.rgb;
	// Specular light.
	specLight += krefl * lightSourceColor * max(0.0, pow(dot(r, eye), specularExponent));

	bottomLight = vec3(0.0, 0.0, 0.0);
	// The light components are added to the total bottom light.
	bottomLight += ambLight;
	bottomLight += diffLight;
	bottomLight += specLight;
	
	// --- Old code ---
	//out_Color = vec4(0.5 * (1 - ktrans) * surfaceLight + ktrans * bottomLight, 1.0);
	// ----------------
	// --- New code ---
	out_Color = vec4(0.5 * surfaceLight + (1 - ktrans) * surfaceColor + ktrans * bottomLight, 1.0);
	// ----------------
}