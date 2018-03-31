#version 430
layout (location = 0) in vec2 vPosition;

layout (binding = 0) uniform sampler2D producer_pos_tex;
layout (binding = 1) uniform sampler2D producer_norm_tex;
layout (binding = 2) uniform sampler2D receiver_pos_tex;

//This is for directional lights, for point lights must be done differenlty
//The process is similar to ray tracing
layout (location = 0) uniform vec3 light_direction;
layout (location = 1) uniform mat4 view_proj;
layout (location = 2) uniform mat4 bias;

out vec2 uv;
out vec3 receiver_pos;

//Constants for refraction
float air_refractive_index = 1;
float glass_refractive_index = 1.517;

//This is from the paper "Caustics Mapping - An Image space technique for Real time Caustics"
vec3 EstimateIntersection (vec3 v, vec3 r, sampler2D posTexture) {
vec3 P1 = v + r;
vec4 texPt = view_proj * vec4(P1, 1.0);
vec2 tc = 0.5 * texPt.xy /texPt.w + vec2(0.5);
vec4 recPos = texture(posTexture, tc);
vec3 P2 = v + distance(v, recPos.xyz) * r;
texPt = view_proj * vec4(P2, 1.0);
tc = 0.5 * texPt.xy /texPt.w + vec2(0.5);
return texture(posTexture, tc).rgb;
}
void main()
{
  //Let us just work with air and glass at the moment
  float ratio = air_refractive_index / glass_refractive_index;

  vec3 refracted = normalize(refract(light_direction, texture(producer_norm_tex, vPosition * 0.5 + vec2(0.5)).rgb, ratio));
  vec3 producer_pos = texture(producer_pos_tex, vPosition * 0.5 + vec2(0.5)).rgb;
  receiver_pos = EstimateIntersection (producer_pos, refracted, receiver_pos_tex);
  //receiver_pos = -refracted;
  //receiver_pos = texture(receiver_pos_tex, vPosition * 0.5 + vec2(0.5)).rgb;
  vec4 temp = bias * view_proj * vec4(receiver_pos, 1.0); 
  gl_Position = temp;
  //gl_Position = vec4(vPosition, 0.0, 1.0);
}