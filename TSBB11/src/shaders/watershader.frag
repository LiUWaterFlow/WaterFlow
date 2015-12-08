#version 430

// ===== Uniform Buffers =====

struct LightParam {
	vec3 pos;
	float isDir;
	vec3 color;
	float specExp;
};

layout(std140, binding = 0) uniform LightInfo {
	LightParam lights[2];
};


// ===== Uniforms =====

uniform vec3 camPos;				// Camera position.

uniform float transparency;
uniform sampler2D terr_texUnit;		// Terrain texture.
uniform sampler2D height_texUnit;	// Terrain normal and height texture.
uniform samplerCube sky_texUnit;	// Skybox texture.

uniform vec3 size;

uniform float time;

// ===== In/Out params =====

in vec3 out_Normal;
in vec3 out_terrNormal;
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
float ktransr;
float ktransg;
float ktransb;
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
	float rhowave1 = sin(0.05 * time + 0.5 + rho1) + 0.2 * sin(-0.01 * time + rho1);
	float rhowave2 = sin(-0.05 * time + 0.5 + rho2/10) + 0.2 * sin(-0.01 * time + rho2 / 10);
 	float rand = 1;
	rand = fract(sin(dot(vec2(out_ObjPos.x + time, out_ObjPos.z), vec2(12.9898, 78.233))) * 43758.5453);
	// Perturbing surface normals.
	Normal = vec3(0.1 * (0.1 * xwave + 0.1 * rhowave1 + rhowave2), out_Normal.y, 0.1 * 0.1 * ywave);
	//Normal += vec3(1 + 0.2 * rand, 1, 1);
	
	Normal = normalize(0.5 * out_Normal + 0.5 * Normal);

	//Normal = out_Normal;

	// Incident and reflected light is calculated for the light source.
	s = normalize(lights[0].pos - (1 - lights[0].isDir) * out_ObjPos);
	r = normalize(2 * Normal * dot(normalize(s), Normal) - s);

	right = cross(Normal, eye);
	// Snell's law
	// Since asin is not that cheap, approximations could be made here.
	theta1 = asin(length(right));
	theta2 = asin(airRefInd * sin(theta1) / waterRefInd);

	// Texture lookup at fragment.
	terrainDataUnderSurface = texture(height_texUnit, out_TexCoord);

	// Depth at fragment.
	depth = out_ObjPos.y - size.y * terrainDataUnderSurface.a;

	// Crude displacement approximation.
	//displacementDirection = normalize(cross(up, right));
	vec3 s2 = airRefInd / waterRefInd * cross(Normal, cross(Normal, eye)) - Normal * sqrt(1 - pow(airRefInd / waterRefInd, 2) * dot(cross(Normal, eye), cross(Normal, eye)));
	displacementDirection = normalize(s2 - dot(s2, -up) * -up);
	bottomDisplacement1 = tan(theta2) * depth;
	displacement1 = bottomDisplacement1 * displacementDirection;

	// Texture lookup at approximation.
	terrainDataAtDis1 = texture(height_texUnit, out_TexCoord + vec2(displacement1.x / size.x, displacement1.z / size.z));

	// To minimize negative depth values, a better approximation is made.
	// "Depth" at approximation.
	depthAtDis1 = out_ObjPos.y - size.y * terrainDataAtDis1.a;
	// Height at approximation (y distance from fragment bottom to approximation bottom).
	h = depth - depthAtDis1;
	// Better approximation
	alpha = abs(atan(h / bottomDisplacement1));
	bottomDisplacement2 = cos(alpha) * depth * sin(theta2) / cos(alpha - theta2);
	displacement2 = bottomDisplacement2 * displacementDirection;

	// Texture lookups at better approximation
	terrainDataAtBottom = texture(height_texUnit, out_TexCoord + vec2(displacement2.x / size.x, displacement2.z / size.z));
	texDataAtBottom = texture(terr_texUnit, out_TexCoord + vec2(displacement2.x / size.x, displacement2.z / size.z));
	// "Depth" at better approximation
	depthAtDis2 = out_ObjPos.y - size.y * terrainDataAtBottom.a;

	// Coordinates and normal of seen position of bottom.
	bottomPos = out_ObjPos + displacement2 - vec3(0, depthAtDis2, 0);
	bottomNormal = terrainDataAtBottom.rgb;

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
	// Transmitted light coefficient.
	// --- Old code ---
	//float transexp = transparency;	// -----Not sure why this is needed.-----
	//ktrans = clamp(pow((1 + wdist), -transparency), 0.0f, 1.0f);
	
	// ----------------
	// --- New code ---
	//float maxDepthColor = clamp(size.y * transparency, 1.0, size.y * transparency);
	//ktrans = clamp(wdist, 0, maxDepthColor);
	//ktrans = 1 - 1 / maxDepthColor * ktrans;
	//surfaceColor = vec3(0.01, 0.02, 0.1);
	// ----------------
	surfaceLight = vec3(0.0, 0.0, 0.0);
	// The light components are added to the total surface light.
	surfaceLight += ambLight;
	//surfaceLight += surfaceColor;
	surfaceLight += reflLight;
	//surfaceLight *= clamp(pow((1 + wdist), -0.1*transparency), 0.0, 1.0);

	ktransr = clamp((1-reflLight.r)*pow((1 + wdist), -1.8*transparency), 0.0, 0.5);
	ktransg = clamp((1-reflLight.g)*pow((1 + wdist), -1.7*transparency), 0.0, 0.5);
	ktransb = clamp((1-reflLight.b)*pow((1 + wdist), -1.5*transparency), 0.0, 0.5);
	kblue = 1 - eye.y;

	//--- Calculating bottomLight

	// Phong lighting for the bottom.
	s = normalize(lights[1].pos - (1 - lights[1].isDir) * bottomPos);
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
	diffLight += kdiff * lights[1].color * max(0.0, dot(s, normalize(bottomNormal)));
	// Specular light.
	specLight += krefl * lights[1].color * max(0.0, pow(dot(r, eye), lights[1].specExp));

	bottomLight = vec3(0.0, 0.0, 0.0);
	// The light components are added to the total bottom light.
	bottomLight += ambLight;
	bottomLight += diffLight;
	bottomLight += specLight;
	bottomLight *= texDataAtBottom.rgb;
	bottomLight = vec3(bottomLight.r * ktransr, bottomLight.g * ktransg, bottomLight.b * ktransb); //Color dependant attenuation

	// --- Old code ---
	//out_Color = vec4(0.5 * (1 - ktrans) * surfaceLight + ktrans * bottomLight, 1.0);
	// ----------------
	// --- New code ---
	out_Color = vec4(surfaceLight + bottomLight, 1.0);
	// ----------------

	// test
	//out_Color = vec4(diffLight, 1.0f);
}
