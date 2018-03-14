#version 430

in vec3 eye_position;
in vec2 texture_coords;
//For debugging
in vec3 eye_tangent;
in vec3 eye_bitangent;
in mat3 TBN;

//To bring world light position to the eye space
uniform mat4 view;

layout(location = 4) uniform vec3 colour;
layout(binding = 0) uniform sampler2D diffuse_texture;
layout(binding = 1) uniform sampler2D normal_texture;

//fixed point light properties - could make them uniform if want to change them
vec3 world_light_position = vec3(0.0, 0.0, 10.0);
vec3 Lspecular = vec3(1.0, 1.0, 1.0);
vec3 Ldiffuse = vec3(1.0, 1.0, 1.0);
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
  vec3 ts_normal = normalize(texture(normal_texture, texture_coords).rgb * 2.0 - 1.0);
  
  vec3 Iambient = Lambient * Kambient;

  vec3 eye_light_position = vec3(view * vec4(world_light_position, 1.0));
  vec3 direction_to_light = normalize(eye_light_position - eye_position);
  vec3 ts_direction_to_light = normalize(TBN * direction_to_light);
  float dot_prod = clamp(dot(ts_direction_to_light, ts_normal), 0.0, 1.0);
  vec3 Idiffuse = Ldiffuse * vec3(texture (diffuse_texture, texture_coords)) * dot_prod;

  vec3 ts_direction_to_viewer = normalize(-TBN * eye_position);
  vec3 ts_half_way = normalize(ts_direction_to_light + ts_direction_to_viewer);
  float dot_prod_specular = clamp(dot(ts_half_way, ts_normal), 0.0, 1.0);
  float specular_factor = pow(dot_prod_specular, specular_exp);
  vec3 Ispecular = Lspecular * Kspecular * specular_factor;

  fColour = vec4(Iambient + Idiffuse + Ispecular, 1.0);
  //fColour = vec4(Iambient, 1.0);
  //fColour = vec4(Idiffuse, 1.0);
  //fColour = vec4(Ispecular, 1.0);
  //fColour = texture (diffuse_texture, texture_coords);
  //fColour = vec4(ts_normal, 1.0);
  //fColour = vec4(dot_prod, dot_prod, dot_prod, 1.0);
  //fColour = vec4(texture_coords.x, texture_coords.y, 0, 1);
  //fColour = vec4(eye_tangent, 1);
  //fColour = vec4(eye_bitangent, 1);
}