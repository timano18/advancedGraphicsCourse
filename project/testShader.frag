#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

out vec4 FragColor;

in vec3 FragPos;    // Position of the fragment
in vec3 Normal;     // Normal vector of the fragment
in vec2 TexCoords;  // Texture coordinates
in vec4 vertexPosition;
in float visibility;
in vec4 clipSpace;

uniform vec3 viewPos;

uniform vec3 lightDirection;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;

layout(binding=0) uniform sampler2D grassTexture;
layout(binding=1) uniform sampler2D stoneTexture;
layout(binding=2) uniform sampler2D sandTexture;
layout(binding=3) uniform sampler2D snowTexture;
layout(binding=4) uniform sampler2D skyTexture;

uniform float materialShininess;


vec3 CalcDirLight(vec3 normal, vec3 viewDir)
{
    vec3 lightDir = -normalize(lightDirection);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);

    // combine results
    vec3 materialColor;

    float snowStart = 400;
    float grassStop = 250;
    float grassStart = 0;
    float sandStop = -100;

    float normalSlopeVal = 0.20f;
    float normalSlopeStrenght = 5.0f;

    // Heights ("ocean" i vertex shader, rakt istället för interpolerat)
    // Snow
    if (FragPos.y > snowStart) {
        materialColor = texture(snowTexture, TexCoords).rgb;
        if (abs(normal.x) > normalSlopeVal || abs(normal.z) > normalSlopeVal) {
            float val = max(abs(normal.x)*normalSlopeStrenght, abs(normal.z)*normalSlopeStrenght) - normalSlopeVal*normalSlopeStrenght;
            val = clamp (val, 0.0, 1.0);
            materialColor = mix(materialColor, texture(stoneTexture, TexCoords).rgb, val);
        }
    } else
    // Interpolate grass to snow
    if (FragPos.y > grassStop) {
        materialColor = mix(texture(grassTexture, TexCoords).rgb, texture(snowTexture, TexCoords).rgb, (FragPos.y-grassStop)/(snowStart-grassStop));
        if (abs(normal.x) > normalSlopeVal || abs(normal.z) > normalSlopeVal) {
            float val = max(abs(normal.x)*normalSlopeStrenght, abs(normal.z)*normalSlopeStrenght) - normalSlopeVal*normalSlopeStrenght;
            val = clamp (val, 0.0, 1.0);
            materialColor = mix(materialColor, texture(stoneTexture, TexCoords).rgb, val);

        }
    } else 
    // Grass
    if (FragPos.y > grassStart) {
        materialColor = texture(grassTexture, TexCoords).rgb;
        if (abs(normal.x) > normalSlopeVal || abs(normal.z) > normalSlopeVal) {
            float val = max(abs(normal.x)*normalSlopeStrenght, abs(normal.z)*normalSlopeStrenght) - normalSlopeVal*normalSlopeStrenght;
            val = clamp (val, 0.0, 1.0);
            materialColor = mix(materialColor, texture(stoneTexture, TexCoords).rgb, val);
        }
    } else
    // Interpolate sand to grass
    if (FragPos.y > sandStop) {
        materialColor = mix(texture(sandTexture, TexCoords).rgb, texture(grassTexture, TexCoords).rgb, (FragPos.y-sandStop)/(grassStart-sandStop));
        if (abs(normal.x) > normalSlopeVal || abs(normal.z) > normalSlopeVal) {
            float val = max(abs(normal.x)*normalSlopeStrenght, abs(normal.z)*normalSlopeStrenght) - normalSlopeVal*normalSlopeStrenght;
            val = clamp (val, 0.0, 1.0);
            materialColor = mix(materialColor, texture(stoneTexture, TexCoords).rgb, val);
        }
    } else 
    // Sand
    if (FragPos.y <= sandStop) {
        materialColor = texture(sandTexture, TexCoords).rgb;
        if (abs(normal.x) > normalSlopeVal || abs(normal.z) > normalSlopeVal) {
            float val = max(abs(normal.x)*normalSlopeStrenght, abs(normal.z)*normalSlopeStrenght) - normalSlopeVal*normalSlopeStrenght;
            val = clamp (val, 0.0, 1.0);
            materialColor = mix(materialColor, texture(stoneTexture, TexCoords).rgb, val);
        }
    }

    vec3 ambient = lightAmbient * materialColor;
    vec3 diffuse = lightDiffuse * diff * materialColor;
    vec3 specular = lightSpecular * spec * materialColor;

    return (ambient + diffuse);
}


void main()
{    
    vec2 normalizedDeviceSpace = (clipSpace.xy/clipSpace.w);
    vec2 screenSpace = normalizedDeviceSpace/2 + 0.5;
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = CalcDirLight(norm, viewDir);
    
    vec4 skyColor = texture(skyTexture, vec2(screenSpace.x, -screenSpace.y));
    vec4 terrainColor = mix(vec4(result,1.0), skyColor,1.0- visibility);
  
    
    
    vec3 fogColor = texture(skyTexture, screenSpace).xyz;
    FragColor = mix(vec4(fogColor, 1.0), vec4(result, 1.0), visibility);
    //FragColor = terrainColor;
    //FragColor = vec4(vec3(result), 1.0f); // combining the two lighting components
}