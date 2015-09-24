#version 150

in vec2 outTexCoord;
uniform sampler2D texUnit;
out vec4 out_Color;

uniform vec2 in_size;

void main(void)
{
	vec4 total_color = texture(texUnit, outTexCoord);
	
	float offset_x = 1.0 / in_size.x;	
	float offset_y = 1.0 / in_size.y;	
	
	vec4 filtered_color;
	
	filtered_color =  4.0 * total_color;
	filtered_color += 2.0 * texture(texUnit, outTexCoord + vec2(0, -offset_y));
	filtered_color += 2.0 * texture(texUnit, outTexCoord + vec2(0, +offset_y));
	filtered_color += 2.0 * texture(texUnit, outTexCoord + vec2(+offset_x, 0));
	filtered_color += 2.0 * texture(texUnit, outTexCoord + vec2(-offset_x, 0));
	filtered_color += 1.0 * texture(texUnit, outTexCoord + vec2(+offset_x, -offset_y));
	filtered_color += 1.0 * texture(texUnit, outTexCoord + vec2(+offset_x, +offset_y));
	filtered_color += 1.0 * texture(texUnit, outTexCoord + vec2(-offset_x, -offset_y));
	filtered_color += 1.0 * texture(texUnit, outTexCoord + vec2(-offset_x, +offset_y));
	
	filtered_color = filtered_color / 16.0;
	
	if(total_color.x < 0.05)
	{
		total_color = filtered_color;
	}

    out_Color = total_color;
}
