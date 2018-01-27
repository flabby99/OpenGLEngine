#version 430
//This is esssentially the same as blinn phong but we restrict the number of shades of colour

in vec3 eye_position;
in vec3 eye_normal;

//To bring world light position to the eye space
uniform mat4 view;

layout(location = 4) uniform vec3 model_colour;
layout(location = 5) uniform float num_shades;
layout(location = 6) uniform vec3 base_colour;

//fixed point light properties - could make them uniform if want to change them
vec3 world_light_position = vec3(0.0, 0.0, 10.0);
vec3 Lspecular = vec3(1.0, 1.0, 1.0);
vec3 Ldiffuse = vec3(0.8, 0.8, 0.8);
vec3 Lambient = vec3(0.15, 0.15, 0.15); //Grey ambience

//surface material reflectance properties - again could be uniforms
float specular_exp = 8.0; 

out vec4 fColour;

//NOTE we could take light attenuation into account for the diffuse and specular.
//In here we compute the intensities and combine them to get a colour
void main() {
	float Iambient = 0.1;
	
	//Diffuse is brightest when light faces surface - dot product
	vec3 eye_light_position = vec3(view * vec4(world_light_position, 1.0));
	vec3 direction_to_light = normalize(eye_light_position - eye_position);
	float Idiffuse = clamp(dot(direction_to_light, eye_normal), 0.0, 1.0);
	
	//Specular takes the angle between the light the surface and the viewer into account
	vec3 direction_to_viewer = normalize(-eye_position);
	vec3 half_way = normalize(direction_to_light + direction_to_viewer);
	float dot_prod_specular = clamp(dot(half_way, eye_normal), 0.0, 1.0);
	float Ispecular = pow(dot_prod_specular, specular_exp);

	//Change from blinn phong
	float intensity = Iambient + Idiffuse + Ispecular;
	float shade_intensity = ceil(intensity * num_shades) / num_shades;

	fColour = vec4(base_colour * model_colour * shade_intensity, 1.0);
}