#version 430

in vec3 texcoords;

layout(binding=0) samplerCube cube_texture;
out vec4 fColour;

void main() {
   fColour = texture(cube_texture, texcoords);
}