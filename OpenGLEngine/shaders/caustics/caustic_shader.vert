#version 430
layout (location = 0) in vec2 vPosition;

layout (binding = 0) uniform sampler2D producer_pos_tex;
layout (binding = 1) uniform sampler2D producer_norm_tex;
layout (binding = 2) uniform sampler2D receiver_pos_tex;
layout (binding = 4) uniform sampler2D tint_tex;

//This is for directional lights, for point lights must be done differenlty
//The process is similar to ray tracing
layout (location = 0) uniform vec3 light_direction;
layout (location = 1) uniform mat4 view_proj;
layout (location = 2) uniform float surface_area;
layout (location = 3) uniform float point_size;

out vec4 intensity;
out vec4 colour;

//Constants for refraction
float air_refractive_index = 1;
float glass_refractive_index = 1.517;
//Let us just work with air and glass at the moment
float ratio = air_refractive_index / glass_refractive_index;

void DistanceEstimationStep(in vec3 v, in vec3 r, in sampler2D posTexture, inout float d) {
  vec3 P1 = v + d * r;
  vec4 texPt = view_proj * vec4(P1, 1.0);
  vec2 tc = 0.5 * texPt.xy /texPt.w + vec2(0.5);
  vec4 recPos = texture(posTexture, tc);
  d = distance(v, recPos.xyz);
}

//This is from the paper "Caustics Mapping - An Image space technique for Real time Caustics"
vec3 EstimateIntersection (in vec3 v, in vec3 r, in sampler2D posTexture) {
  float d = 1.0;
  for(int i = 0; i < 3; ++i) {
    DistanceEstimationStep(v, r, posTexture, d);
  }
  vec3 P2 = v + d * r;
  vec4 texPt = view_proj * vec4(P2, 1.0);
  vec2 tc = 0.5 * texPt.xy /texPt.w + vec2(0.5);
  return texture(posTexture, tc).rgb;
}

void main()
{
  vec4 producer_pos = texture(producer_pos_tex, vPosition * 0.5 + vec2(0.5));
  //Ignore blank positions
  if(producer_pos.a < 0.00001) {
    gl_Position = vec4(0.0);
    intensity = vec4(0.0);
  }
  else {
    vec3 receiver_normal = texture(producer_norm_tex, vPosition * 0.5 + vec2(0.5)).rgb;
    vec3 refracted = normalize(refract(light_direction, receiver_normal, ratio));
    vec3 receiver_pos = EstimateIntersection (producer_pos.rgb, refracted, receiver_pos_tex);
    vec4 temp = view_proj * vec4(receiver_pos, 1.0); 
    gl_Position = temp;
    gl_PointSize = point_size;
    float light_normal_dot = clamp(dot(receiver_normal, -light_direction), 0.0, 1.0);
    float intensity_scale = light_normal_dot;
    //intensity_scale = light_normal_dot;
    intensity = vec4(vec3(intensity_scale), surface_area);
    colour = texture(tint_tex, vPosition * 0.5 + vec2(0.5));
  }
}