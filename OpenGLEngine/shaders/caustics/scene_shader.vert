#version 430

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vt;

out vec3 eye_position;
out vec3 eye_normal;
out vec2 texture_coords;
out vec2 tex_lookup;

layout(location = 0) uniform mat4 view;
layout(location = 1) uniform mat4 proj;
layout(location = 2) uniform mat4 model;
layout(location = 3) uniform mat4 mv_it;
layout(location = 4) uniform mat4 light_view_proj;

void main() {
  texture_coords = vt;
  //eye_normal = vNormal;
  //Transform vertex position into eye co-ordinates
  vec4 world_position = model * vec4(vPosition, 1.0);
  vec4 TexPt = light_view_proj * world_position;
  tex_lookup = 0.5 * TexPt.xy / TexPt.w + vec2(0.5);
  eye_position = vec3(view * world_position); 
  //Transform vertex normal into eye co-ordinates
  //Allows for non-uniform scaling
  eye_normal = normalize(vec3(mv_it * vec4 (vNormal, 0.0))); 
  gl_Position =  proj * vec4 (eye_position, 1.0);
}