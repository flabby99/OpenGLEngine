#version 430

in vec3 eye_position;
in vec3 eye_normal;

//To bring world light position to the eye space
uniform mat4 view;

layout(location = 4) uniform vec3 Kdiffuse;

//fixed point light properties - could make them uniform if want to change them
vec3 world_light_position = vec3(0.0, 0.0, 10.0);
vec3 Ldiffuse = vec3(0.8, 0.8, 0.8);

//surface material reflectance properties - again could be uniforms
vec3 Kambient = vec3(1.0, 1.0, 1.0);

int k = 5;
out vec4 fColour;

//NOTE we could take light attenuation into account for the diffuse and specular.
//In here we compute the intensities and combine them to get a colour
void main() {
	//Diffuse is brightest when light faces surface - dot product
	vec3 eye_light_position = vec3(view * vec4(world_light_position, 1.0));
	vec3 direction_to_light = normalize(eye_light_position - eye_position);
	//Consider negative dot product to be 0
	vec3 direction_to_viewer = normalize(-eye_position);
	float dot_prod_nl = clamp(dot(direction_to_light, eye_normal), 0.0, 1.0);
	float dot_prod_ne = clamp(dot(eye_normal, direction_to_viewer), 0.0, 1.0);
	vec3 Idiffuse = Ldiffuse * Kdiffuse * pow(dot_prod_nl, k + 1) * pow((1 - dot_prod_ne), 1 - k);
	fColour = vec4(Idiffuse, 1.0);
}