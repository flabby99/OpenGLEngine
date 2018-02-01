#version 430

in vec3 eye_position;
in vec3 eye_normal;

//To bring world light position to the eye space
uniform mat4 view;

layout(location = 4) uniform vec3 colour;
layout(binding = 0) uniform samplerCube cube_texture;

out vec4 fColour;

//NOTE we could take light attenuation into account for the diffuse and specular.
//In here we compute the intensities and combine them to get a colour
void main() {
  vec3 eye_incident = normalize(eye_position);
  vec3 reflected = reflect(eye_incident, eye_normal);
  //convert this back into world space from eye space
  reflected = vec3 (inverse (view) * vec4(reflected, 0.0));

  //Currently this is set up for air -> water
  float ratio = 1.0 / 1.3333;
  vec3 refracted = refract (eye_incident, eye_normal, ratio);
  //convert this back into world space from eye space
  refracted = vec3(inverse(view) * vec4(refracted, 0.0));
  fColour =  texture(cube_texture, refracted) + texture(cube_texture, reflected);
}