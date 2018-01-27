#version 430
//The problem here is that X and Y should always be tangent! I need to get this info from the model!

in vec3 eye_position;
in vec3 eye_normal;

//To bring world light position to the eye space
uniform mat4 model;
uniform mat4 view;

layout(location = 4) uniform vec3 Kdiffuse;

//fixed point light properties - could make them uniform if want to change them
vec3 world_light_position = vec3(0.0, 0.0, 10.0);
vec3 Lspecular = vec3(1.0, 1.0, 1.0);
vec3 Ldiffuse = vec3(0.8, 0.8, 0.8);
vec3 Lambient = vec3(0.15, 0.15, 0.15); //Grey ambience

//surface material reflectance properties - again could be uniforms
vec3 Kspecular = vec3(0.5, 0.5, 0.5); //fully reflect specular light
vec3 Kambient = vec3(1.0, 1.0, 1.0);
float specular_exp = 8.0; 

out vec4 fColour;

//NOTE we could take light attenuation into account for the diffuse and specular.
//In here we compute the intensities and combine them to get a colour
void main() {
	//Perhaps could get these using cross prods of eye pos and normal
	vec3 X = vec3(view * vec4(1.0, 0.0, 0.0, 0.0));
	vec3 Y = vec3(view * vec4(0.0, 0.0, -1.0, 0.0));
	//Diffuse is brightest when light faces surface - dot product
	vec3 eye_light_position = vec3(view * vec4(world_light_position, 1.0));
	vec3 direction_to_light = normalize(eye_light_position - eye_position);
	//Consider negative dot product to be 0
	float dot_prod_nl = clamp(dot(direction_to_light, eye_normal), 0.0, 1.0);
	vec3 Idiffuse = Ldiffuse * Kdiffuse * dot_prod_nl;
	
	//Specular takes the angle between the light the surface and the viewer into account
	vec3 direction_to_viewer = normalize(-eye_position);
	vec3 half_way = normalize(direction_to_light + direction_to_viewer);

	//Should I take these in as uniforms?
	float a_x = 0.1;
	float a_y = 0.1;
	float dot_prod_ne = clamp(dot(eye_normal, direction_to_viewer), 0.0, 1.0);
	float dot_prod_hx = clamp(dot(half_way, X), 0.0, 1.0);
	float dot_prod_hy = clamp(dot(half_way, Y), 0.0, 1.0);
	float dot_prod_hn = clamp(dot(half_way, eye_normal), 0.0, 1.0);
	float specular_factor = dot_prod_nl / ((sqrt(dot_prod_nl * dot_prod_ne) * 4 * a_x * a_y));
	float exp = exp(-2 * (pow((dot_prod_hx / a_x), 2) + pow((dot_prod_hy / a_y), 2)) / (1 + dot_prod_hn));
	vec3 Ispecular = Lspecular * Kspecular * specular_factor * exp;

	fColour = vec4(Ispecular * 0.1, 1.0);
	//fColour = vec4(eye_normal, 1.0); - for debug
}