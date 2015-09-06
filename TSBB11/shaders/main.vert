#version 150

in  vec3 inPosition;
in vec2 inTexCoord;
out vec2 texCoord;

uniform mat4 mdlMatrix;
uniform mat4 camMatrix;
uniform mat4 projMatrix;

void main(void)
{
	texCoord = inTexCoord;

	gl_Position = projMatrix * camMatrix * mdlMatrix * vec4(inPosition, 1.0);
}
