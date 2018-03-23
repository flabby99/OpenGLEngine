#version 430
out vec4 fColour;
in vec2 uv;

void main() {
  fColour = vec4(uv, 0.0, 1.0);
}