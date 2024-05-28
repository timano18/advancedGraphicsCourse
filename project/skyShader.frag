#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords; 
in vec4 clipSpace;
in vec3 FresnelCamera;

uniform float waveScale;

layout(binding=0) uniform sampler2D reflectionTexture;
layout(binding=1) uniform sampler2D refractionTexture;
layout(binding=2) uniform sampler2D dudvMap;
layout(binding=3) uniform sampler2D NormalMap;
layout(binding=4) uniform sampler2D DepthMap;

uniform vec3 viewPos;
uniform vec3 lightDirection;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;
uniform float materialShininess;

float distortionScale = 0.025;
float reflectivity = 1.0;

float depthWeight = 250.0;

// Near and far planes (KOLLA SÅ DESSA ÄR RÄTT!)
float nearPlane = 100.0f;
float farPlane = 200000.0f;

// Calculate normals
vec3 CalcDirLight(vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(lightDirection);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);

    vec3 ambient = lightAmbient * 1.0;
    vec3 diffuse = lightDiffuse * diff * 1.0;
    vec3 specular = lightSpecular * spec * 1.0;

    return (ambient + diffuse);
}

// Main
void main() {

    // Transforms
    vec2 normalizedDeviceSpace = (clipSpace.xy/clipSpace.w);
    vec2 screenSpace = normalizedDeviceSpace/2 + 0.5;

    // Fresnel
    vec3 FresnelAngle = normalize(FresnelCamera);
    float FresnelValue = dot(FresnelAngle, vec3(0.0, 1.0, 0.0));
    FresnelValue = pow(FresnelValue, reflectivity);

    // Depth map
    float depth1 = texture(DepthMap, vec2(screenSpace.x, screenSpace.y)).r;
    float bottomDistance = 2.0 * nearPlane * farPlane / (farPlane + nearPlane - (2.0 * depth1 - 1.0) * (farPlane - nearPlane));

    float depth2 = gl_FragCoord.z;
    float surfaceDistance = 2.0 * nearPlane * farPlane / (farPlane + nearPlane - (2.0 * depth2 - 1.0) * (farPlane - nearPlane));

    float waterDepth = bottomDistance - surfaceDistance;

    // Water distortion
    vec2 distortionDUDV = texture(dudvMap, vec2(TexCoords.x * waveScale, TexCoords.y)).rg * 0.1;
    distortionDUDV = TexCoords + vec2(distortionDUDV.x, distortionDUDV.y + waveScale);
    distortionDUDV = (texture(dudvMap, distortionDUDV).rg * 2.0 - 1.0) * distortionScale * clamp(waterDepth/20.0, 0.0, 1.0);

    vec2 reflectionTexCoords = vec2(screenSpace.x, -screenSpace.y) + distortionDUDV;
    vec2 refractionTexCoords = vec2(screenSpace.x, screenSpace.y) + distortionDUDV;

    // Remove water artifacts at screen edges
    refractionTexCoords = clamp(refractionTexCoords, 0.001, 0.999);
    reflectionTexCoords.x = clamp(reflectionTexCoords.x, 0.001, 0.999);
    reflectionTexCoords.y = clamp(reflectionTexCoords.y, -0.999, -0.001);

    vec4 reflectionColour = texture(reflectionTexture, reflectionTexCoords);
    vec4 refractionColour = texture(refractionTexture, refractionTexCoords);

    // Normals
    vec4 normalMapColour = texture(NormalMap, distortionDUDV);
    vec3 normal = vec3(normalMapColour.r * 2.0 - 1.0, normalMapColour.b, normalMapColour.g * 2.0 - 1.0);
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 normalResult = CalcDirLight(norm, viewDir);

    // Print out
    FragColor = mix(reflectionColour, refractionColour, FresnelValue);
    FragColor = mix(FragColor, vec4(0.0, 0.2, 0.6, 1.0), 0.2) + vec4(normalResult, 0.0) * 0.0;
    FragColor.a = clamp(waterDepth/50.0, 0.0, 1.0); // Remove edge around surface

    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    //FragColor.a = clamp(waterDepth/100.0, 0.0, 1.0); // Remove edge around surface
    //FragColor = vec4(waterDepth/100.0);

    //FragColor = mix(reflectionColour, refractionColour, FresnelValue);
    //FragColor = mix(FragColor, refractionColour, clamp(waterDepth/100.0, 0.0, 1.0));
    //FragColor = mix(FragColor, vec4(0.0, 0.2, 0.6, 1.0), 0.2) + vec4(normalResult, 0.0) * 0.0;
}