#version 150

in vec3 Position;
uniform samplerCube cube_texture;
out vec4 out_Color;

void main(void)
{
    out_Color = texture(cube_texture, Position);
}
