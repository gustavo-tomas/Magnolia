// Phong shading model

struct Light
{
	vec3 color;
	float intensity;
	vec3 position;
};

vec3 phong_shading(vec3 surface_normal, vec3 frag_position, vec3 view_position, Light light)
{
	vec3 normal = normalize(surface_normal);
	vec3 light_dir = normalize(light.position - frag_position);

	// Ambient
	vec3 ambient_color = light.color;

	// Diffuse
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 diffuse_color = diff * light.color;

	// Specular
	float specular_strength = 0.5;
	vec3 view_dir = normalize(view_position - frag_position);
	vec3 reflect_dir = reflect(-light_dir, normal);  
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
	vec3 specular_color = specular_strength * spec * light.color;

	// Attenuation
	float dist = length(light.position - frag_position);
	float constant = 1.0;
	float linear = 0.05;
	float quadratic = 0.0075;
	float attenuation = 1.0 / (constant +
							   linear * dist +
							   quadratic * dist * dist);
 
	diffuse_color *= attenuation * light.intensity;
	ambient_color *= attenuation * light.intensity;
	specular_color *= attenuation * light.intensity;

	return ambient_color + diffuse_color + specular_color;
}
