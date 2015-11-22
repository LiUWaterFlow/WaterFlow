#version 150

in vec3 in_Position;
in vec3 in_Normal;		// Vertex-normal.
in vec2 in_TexCoord; //not currently used

out vec3 out_Normal;
out vec2 out_TexCoord;// not currently used
out vec3 out_ObjPos;

//uniform mat4 MTWMatrix;
uniform mat4 WTVMatrix;
uniform mat4 VTPMatrix;

//uniform mat3 iNormalMatrixTrans;

void main(void)
{
	out_Normal = in_Normal;
	out_TexCoord = in_TexCoord; // not currently used
	out_ObjPos = in_Position;

	gl_Position = VTPMatrix * WTVMatrix * vec4(in_Position, 1.0);
}
