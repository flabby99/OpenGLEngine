#version 430

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexture;
layout(location=3) in vec3 vTangent;
layout(location=4) in vec3 vBitangent;

out vec3 eye_position;
out vec2 texture_coords;
out vec3 eye_tangent;
out mat3 TBN;

layout(location = 0) uniform mat4 view;
layout(location = 1) uniform mat4 proj;
layout(location = 2) uniform mat4 model;
layout(location = 3) uniform mat4 mv_it;

void main() {
  texture_coords = vTexture;
  eye_position = vec3(view * model * vec4(vPosition, 1.0)); 
  vec3 eye_normal = normalize(vec3(mv_it * vec4 (vNormal, 0.0))); 
  eye_tangent = normalize(vec3(mv_it * vec4 (vTangent, 0.0))); 
  vec3 eye_bitangent = normalize(vec3(mv_it * vec4 (vBitangent, 0.0))); 
  TBN = transpose(mat3(eye_tangent, eye_bitangent, eye_normal));
  gl_Position =  proj * vec4 (eye_position, 1.0);
  //gl_Position = proj * view * model * vec4(vBitangent, 1.0);
}