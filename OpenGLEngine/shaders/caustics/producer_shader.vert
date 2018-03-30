#version 430
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;

//Todo remove
layout(location = 0) uniform mat4 view;
layout(location = 1) uniform mat4 proj;
layout(location = 2) uniform mat4 model;

out vec4 world_pos;
out vec3 world_normal;

void main()
{
  world_pos = model * vec4(vPosition, 1.0);
  world_normal = vec3(model * vec4(vNormal, 0.0));
  gl_Position = proj * view * world_pos;
}