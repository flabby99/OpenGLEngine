#version 430
in vec4 world_pos;
in vec3 world_normal;

layout(location = 0) out vec4 fColour;
layout(location = 1) out vec4 fNormal;

void main() {
  fColour = world_pos;
  fNormal = vec4(world_normal, 1.0);
  //fNormal - vec4(1.0);
}