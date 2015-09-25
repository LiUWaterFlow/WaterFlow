#version 150

in vec2 outTexCoord;
uniform sampler2D texUnit;
out vec4 out_Color;

void main(void)
{
	vec4 color = texture(texUnit, outTexCoord);
	color = vec4(color.x);
	if(color.x < 0.05f)
	{
		color.x = 0.0f;
		color.y = 1.0f;
		color.z = 0.0f;
	}
		
	
    out_Color = color;
}
