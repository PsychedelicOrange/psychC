#version 460 core

in vec3 normal;
in vec2 texcoord1;
in vec2 texcoord2;

out vec4 FragColor;

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

void main()
{
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
    
    FragColor = vec4(vec3(metalness),1.0f);
}
