#version 150

in vec2 outTexCoord;
uniform sampler2D texUnit;
out vec4 out_Color;

uniform vec2 in_size;

void main(void)
{
	float offsetX = 1.0 / in_size.x;	
	float offsetZ = 1.0 / in_size.y;	
	
	float sobelX = 0;
	float sobelZ = 0;
	
	// Sobel x
	sobelX += 2.0 * texture(texUnit, outTexCoord + vec2(+offsetX, 0)).x;
	sobelX += 1.0 * texture(texUnit, outTexCoord + vec2(+offsetX, +offsetZ)).x;
	sobelX += 1.0 * texture(texUnit, outTexCoord + vec2(+offsetX, -offsetZ)).x;
	sobelX -= 2.0 * texture(texUnit, outTexCoord + vec2(-offsetX, 0)).x;
	sobelX -= 1.0 * texture(texUnit, outTexCoord + vec2(-offsetX, +offsetZ)).x;
	sobelX -= 1.0 * texture(texUnit, outTexCoord + vec2(-offsetX, -offsetZ)).x;
	
	// Sobel y
	sobelZ += 2.0 * texture(texUnit, outTexCoord + vec2(0, +offsetZ)).x;
	sobelZ += 1.0 * texture(texUnit, outTexCoord + vec2(+offsetX, +offsetZ)).x;
	sobelZ += 1.0 * texture(texUnit, outTexCoord + vec2(-offsetX, +offsetZ)).x;
	sobelZ -= 2.0 * texture(texUnit, outTexCoord + vec2(0, -offsetZ)).x;
	sobelZ -= 1.0 * texture(texUnit, outTexCoord + vec2(+offsetX, -offsetZ)).x;
	sobelZ -= 1.0 * texture(texUnit, outTexCoord + vec2(-offsetX, -offsetZ)).x;

	// Combine to normal
	vec3 normal = normalize(vec3(-sobelX,0.02f,-sobelZ));

    out_Color = vec4(normal,1.0f);
}
