// PBR shading model
// Based on the implementation of https://github.com/SaschaWillems/Vulkan-glTF-PBR
// Also from https://github.com/JoeyDeVries/LearnOpenGL

// @TODO: for now this is just a simplified model. We will (hopefully) add some juicy GI in the future and tweak the brdf as necessary

// Lights must be provided by the calling shader
#define NUMBER_OF_LIGHTS u_push_constants.number_of_lights
#define LIGHTS u_lights.lights
#define MIN_VAL 0.0000001 // Mostly to prevent divisions by zero

// -----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return nom / max(denom, MIN_VAL);
}

// -----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / max(denom, MIN_VAL);
}

// -----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// -----------------------------------------------------------------------------
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float attenuation(vec3 light_position, vec3 frag_position)
{
    float dist = length(light_position - frag_position);
    float constant = 1.0;
    float linear = 0.05;
    float quadratic = 0.0075;
    float attenuation = 1.0 / (constant +
                               linear * dist +
                               quadratic * dist * dist);

    return attenuation;
}

vec3 pbr_shading(MaterialData material,
                 vec3 normal,
                 vec3 camera_position,
                 vec3 frag_position,
                 uint debug)
{
    vec3 N = normalize(normal);
	vec3 V = normalize(camera_position - frag_position);
  
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, material.albedo.rgb, material.metallic);

	// Debug
	float total_D = 0;
	vec3  total_F = vec3(0);
	float total_G = 0;
	vec3  total_Specular = vec3(0);

    // Reflectance equation
    vec3 Lo = vec3(0.0);
    for (uint i = 0; i < NUMBER_OF_LIGHTS; i++) 
    {
		// Radiance
        vec3 L = normalize(LIGHTS[i].position - frag_position);
        vec3 H = normalize(V + L);
        float distance = length(LIGHTS[i].position - frag_position);
        float light_attenuation = attenuation(LIGHTS[i].position, frag_position);
        vec3 radiance = LIGHTS[i].color * LIGHTS[i].intensity * light_attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, material.roughness);   
        float G   = GeometrySmith(N, V, L, material.roughness);      
        vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);
           
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + MIN_VAL;
        vec3 specular = numerator / denominator;
        
        vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - material.metallic;	  

        float NdotL = max(dot(N, L), 0.0);        

        Lo += (kD * material.albedo.rgb / PI + specular) * radiance * NdotL;

		total_D += NDF;
		total_F += F;
		total_G += G;
		total_Specular += specular;
    }   
    
    // @TODO: GI goes here
    vec3 ambient = vec3(0.03) * material.albedo.rgb;
    
    vec3 color = ambient + Lo;

	// PBR equation debug visualization
	// "D , "F", "G", Specular"
	switch (debug)
	{
		case 7:
			color.rgb = vec3(total_D);
			break;

		case 8:
			color.rgb = total_F;
			break;

		case 9:
			color.rgb = vec3(total_G);
			break;

		case 10:
			color.rgb = total_Specular;
			break;				
	}

    return color;
}
