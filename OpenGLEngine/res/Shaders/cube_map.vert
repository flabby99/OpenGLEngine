#version 430

layout(location=0) in vec3 vPosition;

//NOTE this view matrix does not include camera translation
//This would be easiest to deal with by keeping a quaternion in the camera which holds the
//Current orientation and then output this as a matrix
layout(location=0) uniform mat4 view;
layout(location=1) uniform mat4 proj;
layout(location=2) uniform mat4 scale;

out vec3 texcoords;

void main() {
    texcoords = vPosition;
    gl_Position = proj * view * scale * vec4(vPosition, 1.0);
}