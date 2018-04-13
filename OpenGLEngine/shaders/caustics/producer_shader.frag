#version 430
in vec4 world_pos;
in vec3 world_normal;
//in vec3 final_colour;

layout(location = 3) uniform vec3 colour;

layout(location = 0) out vec4 fColour;
layout(location = 1) out vec4 fNormal;
layout(location = 2) out vec4 fTint;

void main() {
  fColour = world_pos;
  fNormal = vec4(world_normal, 1.0);
  fTint = vec4(colour, 1.0);
  //fNormal - vec4(1.0);
}