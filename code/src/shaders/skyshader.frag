/// @file skyshader.frag
/// @brief Fragment shader for skybox rendering
///
/// Since the skybox texture is a cube map the skybox rendering is super simple.

#version 150

in vec3 Position; ///< Input position
uniform samplerCube cube_texture; ///< The cube texture (a cube map)
out vec4 out_Color; ///< The output color

void main(void) {
    out_Color = texture(cube_texture, Position);
}
