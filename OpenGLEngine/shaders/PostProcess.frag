#version 430

in vec2 st;

layout(binding = 0) uniform sampler2D tex;

out vec4 frag_colour;

void main() {
  //Invert the colour of the right hand side
  vec3 colour;
  if(st.s >= 0.5) {
    colour = 1.0 - texture(tex, st).rgb;
  } else {
    colour = texture(tex, st).rgb;
  }
  frag_colour = vec4(colour, 1.0);
}