#version 150

in vec2 outTexCoord;
uniform sampler2D dataTex;
uniform sampler2D confTex;
out vec4 out_Color;


void main(void)
{
	vec4 data = texture(dataTex, outTexCoord);
	vec4 conf = texture(confTex, outTexCoord);
	
	if(conf.x > 0.03)
	{
		data.x = data.x / conf.x;
	}
	
	vec4 total_color = vec4(data.x);
	
    out_Color = total_color;
}
