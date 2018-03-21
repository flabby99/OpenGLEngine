#version 430
in vec3 light_world_pos;

out vec4 colour;

void main() {
  colour = vec4(light_world_pos, 1.0);
}