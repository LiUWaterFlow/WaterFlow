/// @file skyshader.frag
/// @brief Fragment shader for skybox rendering
///
/// Since the skybox texture is a cube map the skybox rendering is super simple.

#version 150

in vec3 Position;
uniform samplerCube cube_texture;
out vec4 out_Color;

void main(void)
{
    out_Color = texture(cube_texture, Position);
}
