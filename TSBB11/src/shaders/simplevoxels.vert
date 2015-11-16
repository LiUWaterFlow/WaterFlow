#version 150

in vec3 posValue;

uniform mat4 WTVMatrix;

void main()
{	
	gl_Position = WTVMatrix * vec4(posValue, 1.0f);
}
