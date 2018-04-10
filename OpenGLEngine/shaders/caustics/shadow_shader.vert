#version 430
layout (location = 0) in vec3 vPosition;

layout(location = 0) uniform mat4 view_proj;
layout(location = 2) uniform mat4 model;

void main()
{
 gl_Position = view_proj * model * vec4(vPosition, 1);
}