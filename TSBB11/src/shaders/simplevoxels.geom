/// @file simplevoxels.geom
/// @brief Geometry shader to draw a simple representation of the voxel grid.
///
/// Creates billboards from each point in the voxel grid

#version 150 

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

out vec2 texValue;
out vec3 exNormal;
out vec4 outPosition;

uniform mat4 VTPMatrix;

void main()
{
	// Create the initial positions
	vec3 toEmit = gl_in[0].gl_Position.xyz;
	vec3 toView = vec3(0,0,1);
	vec3 left = normalize(cross(toView, vec3(0,1,0)));
	vec3 top = normalize(cross(left,toView));
	vec4 tempPos;
	float radius = 0.5f;

	// Create the bottom left corner
	toEmit += radius * (left - top);
	tempPos = vec4(toEmit, 1.0f);
	gl_Position = VTPMatrix * tempPos;
	outPosition = tempPos;
	texValue = vec2(0.0f, 0.0f);
	exNormal = toView - top + left;
	EmitVertex();
	
	// Create the bottom right corner
	toEmit -= 2 * radius * left;
	tempPos = vec4(toEmit, 1.0f);
	gl_Position = VTPMatrix * tempPos;
	outPosition = tempPos;
	texValue = vec2(1.0f, 0.0f);
	exNormal = toView - top - left;
	EmitVertex();
	
	// Create the top left corner
	toEmit += 2 * radius * (top + left);
	tempPos = vec4(toEmit, 1.0f);
	gl_Position = VTPMatrix * tempPos;
	outPosition = tempPos;
	texValue = vec2(0.0f, 1.0f);
	exNormal = toView + top + left;
	EmitVertex();
		
	// Create the top right corner
	toEmit -= 2 * radius * left;
	tempPos = vec4(toEmit, 1.0f);
	gl_Position = VTPMatrix * tempPos;
	outPosition = tempPos;
	texValue = vec2(1.0f, 1.0f);
	exNormal = toView + top - left;
	EmitVertex();
	
	EndPrimitive();
}

