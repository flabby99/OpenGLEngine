#version 430

uniform mat4 model_view_matrix;

layout (location = 0) in vec3 position;

out vec3 light_world_pos;

void main()
{
  light_world_pos = vec3(model_view_matrix * vec4(position, 1.0));
  gl_Position = vec4(light_world_pos, 1.0);
}