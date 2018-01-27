#version 430
//This adjusts each position along the normal direction

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;

layout(location = 0) uniform mat4 view;
layout(location = 1) uniform mat4 proj;
layout(location = 2) uniform mat4 model;
layout(location = 3) uniform mat4 mv_it;

layout(location = 5) uniform float offset;

void main() {
	//Transform vertex position into eye co-ordinates
	vec3 eye_position = vec3(view * model * vec4(vPosition, 1.0)); 
	//Transform vertex normal into eye co-ordinates
	//Allows for non-uniform scaling
	vec3 eye_normal = normalize(vec3(mv_it * vec4 (vNormal, 0.0))); 
	gl_Position =  proj * vec4 (eye_position + (offset * eye_normal), 1.0);
}