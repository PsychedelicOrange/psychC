#version 460 core

in vec3 normal;
in vec2 texcoord1;
in vec2 texcoord2;
in vec3 world_pos;

out vec4 FragColor;

uniform vec3 cam_pos;

uniform sampler2D base_color_tex;
uniform sampler2D normal_tex;
uniform sampler2D rough_metal_tex;
uniform sampler2D lightmap_tex;

uniform bool base_color_texture_exists;
uniform bool roughmetal_texture_exists;
uniform bool normal_texture_exists;

uniform vec4 base_color_factor;
uniform float metallic_factor;
uniform float roughness_factor;   

float DistributionGGX(vec3 N, vec3 H, float roughness);

float GeometrySchlickGGX(float NdotV, float roughness);

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);

vec3 fresnelSchlick(float cosTheta, vec3 F0);

vec3 pbr(vec3 albedo, float metallic, float roughness, vec3 light, vec3 V, vec3 N);

void main()
{
	// material setup
    vec4 base_color = base_color_factor; 
    float metalness = metallic_factor;
    float roughness = roughness_factor;

    if(base_color_texture_exists){
		base_color *= texture(base_color_tex, texcoord1);
    }
    if(roughmetal_texture_exists){
		vec4 rmtex = texture(rough_metal_tex, texcoord1);
		roughness *= rmtex.g;
		metalness *= rmtex.b;
    }

	// vectors setup
	vec3 view = normalize(cam_pos - world_pos);

	//lighting 
	// simple diffuse 
	//vec3 light = normalize(vec3(10,0,0)- world_pos);
	//float diffuse_factor = max(dot(light,normal),0);
    //FragColor = vec4(vec3(base_color) * diffuse_factor,1.0f);
	// pbr
	vec3 color = pbr(vec3(base_color), metalness,roughness, vec3(10,5,8), view, normal);
	
    // HDR tonemapping
    //color = color / (color + vec3(1.0));
    // gamma correct
    //color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(vec3(color),1.0f);
}

vec3 pbr(vec3 albedo, float metallic, float roughness, vec3 light, vec3 V, vec3 N){
    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
	vec3 Lo = vec3(0.0);
	vec3 L = normalize(light - world_pos);
	vec3 H = normalize(V + L);
	float distance = length(light - world_pos);
	float attenuation = 1.0 / (distance * distance);
	vec3 radiance = vec3(1,1,1);//  * attenuation; // light color

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(N, H, roughness);   
	float G   = GeometrySmith(N, V, L, roughness);      
	vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

	vec3 numerator    = NDF * G * F; 
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
	vec3 specular = numerator / denominator;

	// kS is equal to Fresnel
	vec3 kS = F;
	// for energy conservation, the diffuse and specular light can't
	// be above 1.0 (unless the surface emits light); to preserve this
	// relationship the diffuse component (kD) should equal 1.0 - kS.
	vec3 kD = vec3(1.0) - kS;
	// multiply kD by the inverse metalness such that only non-metals 
	// have diffuse lighting, or a linear blend if partly metal (pure metals
	// have no diffuse light).
	kD *= 1.0 - metallic;	  

	// scale light by NdotL
	float NdotL = max(dot(N, L), 0.0);        

	// add to outgoing radiance Lo
	Lo += (kD * albedo / 3.14 + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	return Lo;
}
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14 * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
