#version 430
in vec4 world_pos;

out vec4 fColour;

void main() {
  fColour = world_pos;
}