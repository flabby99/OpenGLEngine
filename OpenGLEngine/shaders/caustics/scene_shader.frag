#version 430

in vec3 eye_position;
in vec3 eye_normal;
in vec2 texture_coords;
in vec4 TexPt;

//To bring world light position to the eye space
uniform mat4 view;

layout(location = 5) uniform vec3 colour;
layout(binding = 0) uniform sampler2D diffuse_texture;
layout(binding = 3) uniform sampler2D caustic_intensity;
layout(binding = 4) uniform sampler2DShadow depth_texture;
//layout(binding = 4) uniform sampler2D depth_texture;

//fixed point light properties - could make them uniform if want to change them
layout(location = 6) uniform vec3 world_light_position;
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
  //vec3 Idiffuse = Ldiffuse * vec3(texture (diffuse_texture, texture_coords)) * dot_prod;
  vec3 Idiffuse = Ldiffuse * Kdiffuse * dot_prod * texture (diffuse_texture, texture_coords).rgb;

  //Specular takes the angle between the light the surface and the viewer into account
  vec3 direction_to_viewer = normalize(-eye_position);
  vec3 half_way = normalize(direction_to_light + direction_to_viewer);
  float dot_prod_specular = clamp(dot(half_way, eye_normal), 0.0, 1.0);
  float specular_factor = pow(dot_prod_specular, specular_exp);
  vec3 Ispecular = Lspecular * Kspecular * specular_factor;

  //Shadowing
  //float bias = 0.005*tan(acos(dot_prod));
  //bias = clamp(bias, 0.0, 0.01);
  //float visibility = 1.0;
  //if (texture(depth_texture, TexPt.xy).z < TexPt.z - bias) {
	//visibility = 0.5;
  //}
  float visibility = texture(depth_texture, TexPt.xyz);
  //fColour = vec4(Iambient + Idiffuse + Ispecular, 1.0);
  vec4 temp = texture(caustic_intensity, TexPt.xy);
  if(temp.a < 0.02) {
	temp.a = 0.0;
  }
  fColour = vec4(Iambient + temp.a * temp.rgb + visibility * (Idiffuse + Ispecular), 1.0);
  //fColour = vec4(texture(depth_texture, TexPt.xy).z, 0, 0, 1);
  //fColour = vec4(visibility, 0, 0, 1);
  //fColour = vec4(texture)
}