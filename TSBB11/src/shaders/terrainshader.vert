#version 430

layout (std430,binding = 6) buffer velocity
{
	float v[];
	
};

in vec3 in_Position;
in vec3 in_Normal;		// Vertex-normal.
in vec2 in_TexCoord; //not currently used

out vec3 out_Normal;
out vec2 out_TexCoord;// not currently used
out vec3 out_ObjPos;

uniform mat4 WTVMatrix;
uniform mat4 VTPMatrix;

//out float out_Col_i;
//out float out_Col_j;

//uniform mat4 MTWMatrix;
//uniform mat3 iNormalMatrixTrans;

//uniform ivec2 size;

void main(void)
{
	/*
	int i = int(in_Position.x);
	int j = int(in_Position.z);
	 
	int ipos = clamp(i+1,0,size.x-1);
	int imin = clamp(i-1,0,size.x-1);
	int jpos = clamp(i+1,0,size.x-1);
	int jmin = clamp(i-1,0,size.x-1);
		
	int off0 = i+j*size.x;
	int off1 = imin+j*size.x;
	int off2 = ipos+j*size.x;
	int off3 = i+jmin*size.x;
	int off4 = i+jpos*size.x;
	
	
	out_Col_i = (v[off0]-v[off1]) + (v[off0] -v[off2]); 
	out_Col_j = (v[off0]-v[off3]) + (v[off0] -v[off4]); 
	*/
	
	out_Normal = in_Normal;
	out_TexCoord = in_TexCoord; // not currently used
	out_ObjPos = in_Position;

	gl_Position = VTPMatrix * WTVMatrix * vec4(in_Position, 1.0);
}
