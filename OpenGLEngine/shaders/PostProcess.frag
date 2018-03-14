#version 430

in vec2 st;

layout(binding = 0) uniform sampler2D tex;

out vec4 frag_colour;

void main() {
  //Red colour for now
  frag_colour = vec4(1.0, 0.0, 0.0, 1.0);
}