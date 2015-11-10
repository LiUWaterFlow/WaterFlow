#version 150

in vec2 outTexCoord;
uniform sampler2D texUnit;
out vec4 out_Color;

uniform vec2 in_size;
uniform float in_sample;

void main(void)
{
	float offsetX = 1.0 / in_size.x;	
	float offsetZ = 1.0 / in_size.y;	
	
	float sobelX = 0;
	float sobelZ = 0;
	
	// Sobel x
	sobelX += 2.0 * texture(texUnit, outTexCoord + vec2(+offsetX, 0)).y;
	sobelX += 1.0 * texture(texUnit, outTexCoord + vec2(+offsetX, +offsetZ)).y;
	sobelX += 1.0 * texture(texUnit, outTexCoord + vec2(+offsetX, -offsetZ)).y;
	sobelX -= 2.0 * texture(texUnit, outTexCoord + vec2(-offsetX, 0)).y;
	sobelX -= 1.0 * texture(texUnit, outTexCoord + vec2(-offsetX, +offsetZ)).y;
	sobelX -= 1.0 * texture(texUnit, outTexCoord + vec2(-offsetX, -offsetZ)).y;
	
	// Sobel z
	sobelZ += 2.0 * texture(texUnit, outTexCoord + vec2(0, +offsetZ)).y;
	sobelZ += 1.0 * texture(texUnit, outTexCoord + vec2(+offsetX, +offsetZ)).y;
	sobelZ += 1.0 * texture(texUnit, outTexCoord + vec2(-offsetX, +offsetZ)).y;
	sobelZ -= 2.0 * texture(texUnit, outTexCoord + vec2(0, -offsetZ)).y;
	sobelZ -= 1.0 * texture(texUnit, outTexCoord + vec2(+offsetX, -offsetZ)).y;
	sobelZ -= 1.0 * texture(texUnit, outTexCoord + vec2(-offsetX, -offsetZ)).y;

	// Height scaling (0.005f is an  arbitrary value)
	float heightScale = 0.005f * in_sample;

	// Combine to normal
	vec3 normal = normalize(vec3(-sobelX, heightScale, -sobelZ));

    out_Color = vec4(normal,texture(texUnit, outTexCoord).y);
}
