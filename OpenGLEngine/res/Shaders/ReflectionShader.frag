#version 430

in vec3 eye_position;
in vec3 eye_normal;

//To bring world light position to the eye space
uniform mat4 view;

layout(location = 4) uniform vec3 colour;
layout(binding = 0) uniform samplerCube cube_texture;
layout(location = 5) uniform int type;
layout(location = 6) uniform float off = 0.03;

float air_refraction_index = 1;
float glass_refraction_index = 1.517;

out vec4 fColour;

float schlick_fresnel(vec3 I, vec3 N) {
    float r_0 = pow((air_refraction_index - glass_refraction_index) / (air_refraction_index + glass_refraction_index), 2);
    return r_0 + (1 - r_0) * pow(1 - dot(I, N), 5);
}

//REFERENCE See http://developer.download.nvidia.com/CgTutorial/cg_tutorial_chapter07.html
float empirical_fresnel(vec3 I, vec3 N, vec3 f_v) {
    float bias = f_v.x;
    float scale = f_v.y;
    float power = f_v.z;
    return max(0, min(1, bias + scale * pow(1.0 + dot(I,N), power)));
}

void main() {
  vec3 eye_incident = normalize(eye_position);  
  //Do reflections
  vec3 I = vec3(inverse (view) * vec4(eye_incident, 0.0));
  vec3 N = vec3(inverse (view) * vec4(eye_normal, 0.0));
  vec3 reflected = reflect(I, N);
  vec3 reflectedColour = texture(cube_texture, reflected).rgb;

  //Currently this is set up for air -> glass
  float ratio = air_refraction_index / glass_refraction_index;
  //Chromatic dispersion refraction
  float offset = off;
  //The refraction indices of most transparent materials decrease with increasing wavelength lambda
  vec3 Tr = refract(I, N, ratio - offset);
  vec3 Tg = refract(I, N, ratio);
  vec3 Tb = refract(I, N, ratio + offset);
  vec3 refractedColour;
  refractedColour.r = texture(cube_texture, Tr).r;
  refractedColour.g = texture(cube_texture, Tg).g;
  refractedColour.b = texture(cube_texture, Tb).b;

  float fresnel = clamp(empirical_fresnel(I, N, vec3(0.05f, 1.0f, 0.8f)), 0, 1);
  
  //The whole shibang
  if (type == 0)
  fColour =  vec4((reflectedColour * fresnel) + (refractedColour * (1.0f - fresnel)), 1.0f);
  //Just reflection
  else if (type == 1)
  fColour = vec4(reflectedColour, 1.0f);
  //Just refraction
  else if (type == 2) 
  fColour = vec4(texture(cube_texture, refract(I, N, ratio)).rgb, 1.0f);
  //Just Chromatic refraction
  else if (type == 3)
  fColour = vec4(refractedColour, 1.0f);
  //Visualising fresnel
  else if (type == 4)
  fColour = vec4(fresnel, 0.0f, 0.0f, 1.0f);
}