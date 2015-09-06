#version 150

out vec4 outColor;
in vec2 texCoord;
uniform float t;

void main(void)
{
	float a = sin(texCoord.s * 30.0 + t)/2.0 + 0.5;
	float b = sin(texCoord.t * 30.0 * (1.0+sin(t/4.0)))/2.0 + 0.5;
	outColor = vec4(a, b, 0.8, 1.0); // inColor;
}

