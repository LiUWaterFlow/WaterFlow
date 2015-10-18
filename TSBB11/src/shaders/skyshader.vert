#version 150

in vec3 in_Position;
in vec2 in_TexCoord;

out vec2 out_TexCoord;

uniform mat4 VTPMatrix;


void main(void)
{
	out_TexCoord = in_TexCoord;

	gl_Position = VTPMatrix * vec4(in_Position, 1.0);
}
