#version 430

in vec3 eye_position;
in vec3 eye_normal;

//To bring world light position to the eye space
uniform mat4 view;

//fixed point light properties - could make them uniform if want to change them
vec3 world_light_position = vec3(0.0, 0.0, 10.0);
vec3 Lspecular = vec3(1.0, 1.0, 1.0);
vec3 Ldiffuse = vec3(0.8, 0.8, 0.8);
vec3 Lambient = vec3(0.4, 0.2, 0.15); //Maroon ambience

//surface material reflectance properties - again could be uniforms
vec3 Kspecular = vec3(1.0, 1.0, 1.0); //fully reflect specular light
vec3 Kdiffiuse = vec3(1.0, 0.7, 0.1); //Orange diffuse reflectance
vec3 Kambient = vec3(1.0, 1.0, 1.0);
float specular_exp = 4.0; 

out vec4 fColour;

//In here we compute the intensities and combine them to get a colour
void main() {
	//vec3 Iambient = vec3(0.0,0.0,0.0);
	vec3 Iambient = Lambient * Kambient;
	
	//Diffuse is brightest when light faces surface - dot product
	vec3 eye_light_position = vec3(view * vec4(world_light_position, 1.0));
	vec3 direction_to_light = normalize(eye_light_position - eye_position);
	//Consider negative dot product to be 0
	float dot_prod = max(dot(direction_to_light, eye_normal), 0.0);
	//vec3 Idiffuse = vec3(0.0,0.0,0.0);
	vec3 Idiffuse = Ldiffuse * Kdiffiuse * dot_prod;
	
	//Specular takes the angle between the light the surface and the viewer into account
	vec3 eye_reflection = reflect(-direction_to_light, eye_normal);
	vec3 direction_to_viewer = normalize(-eye_position);
	float dot_prod_specular = max(dot(eye_reflection, direction_to_viewer), 0.0);
	float specular_factor = pow(dot_prod_specular, specular_exp);
	//vec3 Ispecular = vec3(0.0,0.0,0.0);
	vec3 Ispecular = Lspecular * Kspecular * specular_factor;
	fColour = vec4(Iambient + Idiffuse + Ispecular, 1.0);
	//fColour = vec4(eye_normal, 1.0); - for debug
}