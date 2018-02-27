#version 430

in vec3 eye_position;
in vec3 eye_normal;
in vec2 texture_coords;

//To bring world light position to the eye space
uniform mat4 view;

layout(location = 4) uniform vec3 colour;
layout(binding = 0) uniform sampler2D diffuse_texture;

//fixed point light properties - could make them uniform if want to change them
vec3 world_light_position = vec3(0.0, 10.0, 10.0);
vec3 Lspecular = vec3(1.0, 1.0, 1.0);
vec3 Ldiffuse = vec3(0.8, 0.8, 0.8);
vec3 Lambient = vec3(0.15, 0.15, 0.15); //Grey ambience

//surface material reflectance properties - again could be uniforms
vec3 Kdiffuse = colour;
vec3 Kspecular = vec3(0.5, 0.5, 0.5); //fully reflect specular light
vec3 Kambient = vec3(1.0, 1.0, 1.0);
float specular_exp = 8.0; 

out vec4 fColour;

//NOTE we could take light attenuation into account for the diffuse and specular.
//In here we compute the intensities and combine them to get a colour
void main() {
  vec3 Iambient = Lambient * Kambient;
  
  //Diffuse is brightest when light faces surface - dot product
  vec3 eye_light_position = vec3(view * vec4(world_light_position, 1.0));
  vec3 direction_to_light = normalize(eye_light_position - eye_position);
  //Consider negative dot product to be 0
  float dot_prod = clamp(dot(direction_to_light, eye_normal), 0.0, 1.0);
  vec3 Idiffuse = Ldiffuse * vec3(texture (diffuse_texture, texture_coords)) * dot_prod;

  //Specular takes the angle between the light the surface and the viewer into account
  vec3 direction_to_viewer = normalize(-eye_position);
  vec3 half_way = normalize(direction_to_light + direction_to_viewer);
  float dot_prod_specular = clamp(dot(half_way, eye_normal), 0.0, 1.0);
  float specular_factor = pow(dot_prod_specular, specular_exp);
  vec3 Ispecular = Lspecular * Kspecular * specular_factor;
  fColour = vec4(Iambient + Idiffuse + Ispecular, 1.0);
  //fColour = vec4(colour, 1.0);
  //fColour = vec4(Idiffuse, 1.0);
  //Debugs
  //fColour = vec4(eye_normal, 1.0);
  //fColour = vec4(texture_coords.x, texture_coords.y, 0, 1);
  //fColour = (texture(diffuse_texture, texture_coords));
  //fColour = glm::vec4(dot_prod, 0.0f, 0.0f, 1.0f);
}