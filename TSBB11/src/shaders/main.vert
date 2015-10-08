#version 150

in vec3 in_Position;
in vec3 in_Normal;		// Vertex-normal.
in vec2 in_TexCoord;

out vec3 out_Normal;
out vec2 out_TexCoord;
out vec3 out_ObjPos;

uniform mat4 MTWMatrix;
uniform mat4 WTVMatrix;
uniform mat4 VTPMatrix;

uniform mat3 iNormalMatrixTrans;

void main(void)
{
	out_Normal = iNormalMatrixTrans*in_Normal;
	out_TexCoord = in_TexCoord;
	out_ObjPos = vec3(MTWMatrix * vec4(in_Position, 1));

	gl_Position = VTPMatrix * WTVMatrix * MTWMatrix * vec4(in_Position, 1.0);
}
