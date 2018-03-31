#version 430
out vec4 fColour;
in vec2 uv;
in vec3 receiver_pos;

void main() {
  //fColour = vec4(receiver_pos, 1.0);
  //fColour = vec4(uv, 0.0, 1.0)
  //TODO change for intensity
  fColour  = vec4(1, 1, 1, 1);
}