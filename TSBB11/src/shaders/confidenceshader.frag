#version 150

in vec2 outTexCoord;
uniform sampler2D texUnit;
out vec4 out_Color;


void main(void)
{
	vec4 total_color = texture(texUnit, outTexCoord);
	
	total_color.x = total_color.x - 0.05;
	total_color.x = ceil(total_color.x);
	
    out_Color = total_color;
}
