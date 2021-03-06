#version 430
out vec4 fColour;
in vec4 intensity;
in vec4 colour;
layout (binding = 4) uniform sampler2D sprite;

void main() {
  vec4 sprite_value = texture(sprite, gl_PointCoord);
  fColour = vec4(intensity.rgb * sprite_value.rgb * colour.rgb, intensity.a);
  //fColour = sprite_value;
}