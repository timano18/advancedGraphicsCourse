#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

out vec4 FragColor;

in vec3 FragPos;    // Position of the fragment
in vec3 Normal;     // Normal vector of the fragment
in vec2 TexCoords;  // Texture coordinates

uniform vec3 viewPos;

uniform vec3 lightDirection;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;

layout(binding=0) uniform sampler2D grassTexture;
layout(binding=1) uniform sampler2D stoneTexture;
layout(binding=2) uniform sampler2D sandTexture;
layout(binding=3) uniform sampler2D snowTexture;

uniform float materialShininess;

vec3 CalcDirLight(vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(lightDirection);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);

    // combine results
    vec3 materialColor;

    float snowStart = 225;
    float grassStop = 200;
    float grassStart = 75;
    float sandStop = 50;

    // Heights ("ocean" i vertex shader, rakt istället för interpolerat)
    // Snow
    if (FragPos.y > snowStart) {
        materialColor = texture(snowTexture, TexCoords).rgb;
        if (abs(Normal.x) > 0.40) materialColor = mix(materialColor, texture(stoneTexture, TexCoords).rgb, 0.5);
    } else
    // Interpolate grass to snow
    if (FragPos.y > grassStop) {
        materialColor = mix(texture(grassTexture, TexCoords).rgb, texture(snowTexture, TexCoords).rgb, (FragPos.y-grassStop)/(snowStart-grassStop));
        if (abs(Normal.x) > 0.40) materialColor = mix(materialColor, texture(stoneTexture, TexCoords).rgb, 0.5);
    } else 
    // Grass
    if (FragPos.y > grassStart) {
        materialColor = texture(grassTexture, TexCoords).rgb;
        if (abs(Normal.x) > 0.40) materialColor = mix(materialColor, texture(stoneTexture, TexCoords).rgb, 0.5);
    } else
    // Interpolate sand to grass
    if (FragPos.y > sandStop) {
        materialColor = mix(texture(sandTexture, TexCoords).rgb, texture(grassTexture, TexCoords).rgb, (FragPos.y-sandStop)/(grassStart-sandStop));
        if (abs(Normal.x) > 0.40) materialColor = mix(materialColor, texture(stoneTexture, TexCoords).rgb, 0.5);
    } else 
    // Sand
    if (FragPos.y <= sandStop) {
        materialColor = texture(sandTexture, TexCoords).rgb;
        if (abs(Normal.x) > 0.40) materialColor = mix(materialColor, texture(stoneTexture, TexCoords).rgb, 0.5);
    }
    // Slope
    //if (abs(Normal.x) > 0.25) {
    //    materialColor = texture(stoneTexture, TexCoords).rgb/0.8;
    //}
    //if (abs(Normal.x) > 0.40) {
    //    materialColor = texture(stoneTexture, TexCoords).rgb;
    //}
    

    vec3 ambient = lightAmbient * materialColor;
    vec3 diffuse = lightDiffuse * diff * materialColor;
    vec3 specular = lightSpecular * spec * materialColor;

    return (ambient + diffuse);
}

void main()
{    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = CalcDirLight(norm, viewDir);

    FragColor = vec4(vec3(result), 1.0f); // combining the two lighting components
}

