#version 150

in vec3 in_Position;

uniform mat4 VTPMatrix;
uniform mat4 WTVMatrix;

out vec3 Position;


void main(void)
{
	Position = in_Position;
	gl_Position = VTPMatrix * vec4(mat3(WTVMatrix) * in_Position, 1.0);
}
