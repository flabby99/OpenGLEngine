#version 430
//The purpose of this shader is to colour the silhoutte of a model black.

out vec4 fColour;

layout(location = 4) uniform vec3 const_colour; 

void main() {
  //Note we can get the z buffer value using gl_Fragcoord.z
  fColour = vec4(const_colour, 1.0);
}