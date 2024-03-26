#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

///////////////////////////////////////////////////////////////////////////////
// Material
///////////////////////////////////////////////////////////////////////////////
uniform vec3 material_color;
uniform float material_metalness;
uniform float material_fresnel;
uniform float material_shininess;
uniform vec3 material_emission;

uniform int has_color_texture;
layout(binding = 0) uniform sampler2D colorMap;

///////////////////////////////////////////////////////////////////////////////
// Environment
///////////////////////////////////////////////////////////////////////////////
layout(binding = 6) uniform sampler2D environmentMap;
layout(binding = 7) uniform sampler2D irradianceMap;
layout(binding = 8) uniform sampler2D reflectionMap;
uniform float environment_multiplier;

///////////////////////////////////////////////////////////////////////////////
// Light source
///////////////////////////////////////////////////////////////////////////////
uniform vec3 point_light_color = vec3(1.0, 1.0, 1.0);
uniform float point_light_intensity_multiplier = 50.0;


///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////
#define PI 3.14159265359

///////////////////////////////////////////////////////////////////////////////
// Input varyings from vertex shader
///////////////////////////////////////////////////////////////////////////////
in vec2 texCoord;
in vec3 viewSpaceNormal;
in vec3 viewSpacePosition;

///////////////////////////////////////////////////////////////////////////////
// Input uniform variables
///////////////////////////////////////////////////////////////////////////////
uniform mat4 viewInverse;
uniform vec3 viewSpaceLightPosition;

///////////////////////////////////////////////////////////////////////////////
// Output color
///////////////////////////////////////////////////////////////////////////////
layout(location = 0) out vec4 fragmentColor;




vec3 calculateDirectIllumiunation(vec3 wo, vec3 n, vec3 base_color)
{
	vec3 direct_illum = base_color;
	///////////////////////////////////////////////////////////////////////////
	// Task 1.2 - Calculate the radiance Li from the light, and the direction
	//            to the light. If the light is backfacing the triangle,
	//            return vec3(0);
	///////////////////////////////////////////////////////////////////////////
	
	float d = length(viewSpaceLightPosition - viewSpacePosition);
	
	vec3 Li = point_light_intensity_multiplier * 1000 * point_light_color * (1 / (d * d));
	
	vec3 wi = normalize(viewSpaceLightPosition - viewSpacePosition);
	

	
	if(dot(n, wi) <=  0.0)
		return vec3(0.0);

		

	
	///////////////////////////////////////////////////////////////////////////
	// Task 1.3 - Calculate the diffuse term and return that as the result
	///////////////////////////////////////////////////////////////////////////
	vec3 diffuse_term = base_color * (1.0 / PI) * dot(n, wi) * Li;
	direct_illum = diffuse_term;

		
	
	///////////////////////////////////////////////////////////////////////////
	// Task 2 - Calculate the Torrance Sparrow BRDF and return the light
	//          reflected from that instead
	///////////////////////////////////////////////////////////////////////////
	//float fresnel_term = R0 + (1-R0)*(1-wh*wi)^5
	vec3 wh = normalize(wi * wo);
	float nwh = max(0.0001, dot(n, wh));
	float nwo = max(0.0001, dot(n, wo));
	float nwi = max(0.0001, dot(n, wi));
	float wowh = max(0.0001, dot(wo, wh));
	float whwi = max(0.0001, dot(wh, wi));
	float F = material_fresnel + (1.0 - material_fresnel) * pow(1.0 - whwi, 5.0);
	float D = (material_shininess + 2.0) * (1.0/(2.0*PI)) * pow(nwh,material_shininess);
	float G = min(1.0, min(2.0  * nwh * nwo * (1.0 / wowh), 2.0  * nwh * nwi * (1.0 / wowh)));

	float denominator = 4.0 * clamp(nwo * nwi, 0.0001, 1.0); 
	float brdf = F * D * G / denominator;


	///////////////////////////////////////////////////////////////////////////
	// Task 3 - Make your shader respect the parameters of our material model.
	///////////////////////////////////////////////////////////////////////////
	vec3 dielectric_term = brdf * nwi * Li + (1.0 - F) * diffuse_term;
	vec3 metal_term = brdf * base_color * nwi * Li;
	direct_illum = material_metalness * metal_term + (1.0 - material_metalness) * dielectric_term; 

//	direct_illum = vec3(G);
	return direct_illum;
}

vec3 calculateIndirectIllumination(vec3 wo, vec3 n, vec3 base_color)
{
	vec3 indirect_illum = vec3(0.f); 
	
	//return indirect_illum;
	///////////////////////////////////////////////////////////////////////////
	// Task 5 - Lookup the irradiance from the irradiance map and calculate
	//          the diffuse reflection
	///////////////////////////////////////////////////////////////////////////
	// Calculate the spherical coordinates of the direction
	vec3 world_normal = vec3(viewInverse * vec4(n, 0.0));
	
	float theta = acos(max(-1.0f, min(1.0f, world_normal.y)));
	float phi = atan(world_normal.z, world_normal.x);
	if(phi < 0.0f)
	{
		phi = phi + 2.0f * PI;
	}
	
	vec2 lookup = vec2(phi / (2.0 * PI), 1 - theta / PI);
	vec3 Li = environment_multiplier * texture(irradianceMap, lookup).rgb;
	vec3 diffuse_term = base_color * (1.0 / PI) * Li;
	indirect_illum = diffuse_term;
	
	///////////////////////////////////////////////////////////////////////////
	// Task 6 - Look up in the reflection map from the perfect specular
	//          direction and calculate the dielectric and metal terms.
	///////////////////////////////////////////////////////////////////////////
	vec3 wi = normalize(reflect(-wo, n));
	vec3 wr = normalize(vec3(viewInverse * vec4(wi, 0.0)));
	theta = acos(max(-1.0f, min(1.0f, wr.y)));
	phi = atan(wr.z, wr.x);
	if(phi < 0.0f)
	{
		phi = phi + 2.0f * PI;
	}
	
	lookup = vec2(phi / (2.0 * PI), 1 - theta / PI);
	
	float roughness = sqrt(sqrt(2.0 / (material_shininess + 2.0)));
	Li = environment_multiplier * textureLod(reflectionMap, lookup, roughness * 7.0).rgb;

	vec3 wh = normalize(wi + wo);
	float wodotwh = max(0.0, dot(wo, wh));
	float F = material_fresnel + (1.0 - material_fresnel) * pow(1.0 - wodotwh, 5.0);
	vec3 dielectric_term = F * Li + (1.0 - F) * diffuse_term;
	vec3 metal_term = F * base_color * Li;

	vec3 microfacet_term = material_metalness * metal_term + (1.0 - material_metalness) * dielectric_term;

	indirect_illum = microfacet_term;

	return indirect_illum;
}

void main()
{
	///////////////////////////////////////////////////////////////////////////
	// Task 1.1 - Fill in the outgoing direction, wo, and the normal, n. Both
	//            shall be normalized vectors in view-space.
	///////////////////////////////////////////////////////////////////////////
	vec3 n = normalize(viewSpaceNormal);
	vec3 wo = -normalize(viewSpacePosition);

	

	vec3 base_color = material_color;
	
	if(has_color_texture == 1)
	{
		base_color *= texture(colorMap, texCoord).rgb;
	}
	

	vec3 direct_illumination_term = vec3(0.0);
	{ // Direct illumination
		direct_illumination_term = calculateDirectIllumiunation(wo, n, base_color);
	}
	fragmentColor.rgb = direct_illumination_term;
	return;

	vec3 indirect_illumination_term = vec3(0.0);
	{ // Indirect illumination
		indirect_illumination_term = calculateIndirectIllumination(wo, n, base_color);
	}
	fragmentColor.rgb = indirect_illumination_term;
	//return;
	///////////////////////////////////////////////////////////////////////////
	// Task 1.4 - Make glowy things glow!
	///////////////////////////////////////////////////////////////////////////
	vec3 emission_term = material_emission;

	vec3 final_color = direct_illumination_term + indirect_illumination_term + emission_term;

	// Check if we got invalid results in the operations
	if(any(isnan(final_color)))
	{
		final_color.rgb = vec3(1.f, 0.f, 1.f);
	}

	fragmentColor.rgb = final_color;


	



	/* 
	//Temp shading
	// ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * point_light_color;
  	
    // diffuse 
    vec3 norm = normalize(viewSpaceNormal);
    vec3 lightDir = normalize(viewSpaceLightPosition - viewSpacePosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * point_light_color;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - viewSpacePosition);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * point_light_color;  
        
    vec3 result = (ambient + diffuse + specular) * material_color;
    fragmentColor = vec4(result   , 1.0);
	*/
	//fragmentColor = vec4(shading, 1.0);
	return;
}
