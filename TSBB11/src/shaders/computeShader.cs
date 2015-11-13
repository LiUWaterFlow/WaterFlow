#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout (shared,binding = 0) buffer Pos
{
	float position[];
	
}inPos;

layout (shared, binding = 1) buffer Normal
{
	float normal[];
	
}inNormal;

layout (shared,binding = 2) buffer TexCoord
{
	vec2 texCoord[];
	
}inTexCoord;

layout (shared,binding = 3) buffer index
{	
	int indices[];
	
}inIndex;

uniform sampler2D tex;

uniform ivec2 size; //width height

void main(){
	//determine where to sample
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);

	if(storePos.x < size.x && storePos.y < size.y) {
		int offset = (storePos.x + storePos.y*size.x)*3;
		//read from texture;
		vec4 texData = texture(tex,vec2(float(storePos.x)/float(size.x),float(storePos.y)/float(size.y)));
		//write to position (x,z) from where we read, y is data we read.
		inPos.position[offset] = float(storePos.x)/float(size.x);
		inPos.position[offset+1] = texData.a;
		inPos.position[offset+2] = float(storePos.y)/float(size.y);
		//normal is in tex
		inNormal.normal[offset] = texData.r;
		inNormal.normal[offset+1] = texData.g;
		inNormal.normal[offset+2] = texData.b;
		//texCoord = x/width, z/height (typ)
		inTexCoord.texCoord[offset/3] = vec2(float(storePos.x)/float(size.x), float(storePos.y)/float(size.y));
		//problem remaining is indices. 
		if((storePos.x != 0 && storePos.y != 0)){
			int vert1 = (storePos.x-1) + (storePos.y-1)*size.x;
			int offsetIndex = vert1*6; 
			int vert2 = (storePos.x-1) + (storePos.y)*size.x;
			int vert3 = (storePos.x + storePos.y*size.x);
			int vert4 = (storePos.x) + (storePos.y-1)*size.x;
			inIndex.indices[offsetIndex] = vert4;
			inIndex.indices[offsetIndex +1] = vert1;
			inIndex.indices[offsetIndex +2] = vert3;

			inIndex.indices[offsetIndex +3] = vert1;
			inIndex.indices[offsetIndex +4] = vert2;
			inIndex.indices[offsetIndex +5] = vert3;
		}
	}
}
