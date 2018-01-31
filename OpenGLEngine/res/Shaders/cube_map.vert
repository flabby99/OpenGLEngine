#version 430

layout(location=0) in vec3 vPosition;

//NOTE this view matrix does not include camera translation
layout(location=0) uniform mat4 view;
layout(location=1) uniform mat4 proj;

out vec3 texcoords;

void main() {
    texcoords = vPosition;
    gl_Position = proj * view * vec4(vPosition, 1.0);
}