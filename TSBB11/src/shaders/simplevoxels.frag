#version 150

in vec4 outPosition;
in vec3 exNormal;

out vec4 outColor;

uniform mat4 WTVMatrix;

void main()
{
	vec3 newColor;
	
	// Calculate light
	vec3 light = vec3(0.0f, 1.0f, 0.0f);
	float shade = max(dot(normalize(exNormal), mat3(worldView) * light), 0.1);
	
	// Get texture colors
	vec4 texColor = texture(texUnit, texValue);
	
	// Output complete color
	outColor =  texColor * vec4(foggedColor * shade, 0.2f);
	
	if(texColor.r < 0.85 && texColor.g < 0.85 && texColor.b < 0.85)
		discard;
}