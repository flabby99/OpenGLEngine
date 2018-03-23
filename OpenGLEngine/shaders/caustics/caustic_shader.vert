#version 430
layout (location = 0) in vec2 vPosition;

//Todo remove
layout(location = 0) uniform mat4 view;
layout(location = 1) uniform mat4 proj;
layout(location = 2) uniform mat4 model;

out vec2 uv;

void main()
{
  uv = vPosition;
  gl_Position = vec4(vPosition, 0.0, 1.0);
}