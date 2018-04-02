#version 430
out vec4 fColour;
in vec4 intensity;

void main() {
  fColour  = intensity;
}