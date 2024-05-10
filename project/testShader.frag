#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

out vec4 FragColor;

in vec3 FragPos;    // Position of the fragment
in vec3 Normal;     // Normal vector of the fragment
in vec2 TexCoords;  // Texture coordinates

in vec3 vertColour;

uniform vec3 viewPos;

uniform vec3 lightDirection;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;

uniform float materialShininess;
uniform vec3 materialColor1;
uniform vec3 materialColor2;
uniform vec3 materialColor3;
uniform vec3 materialColor4;
uniform vec3 materialColor5;

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

    /*
    // Heights
    // Snow
    if (FragPos.y > 300) {
        materialColor = materialColor5;
    } else 
    // Grass
    if (FragPos.y > 50) {
        materialColor = materialColor1;
    } else
    // Sand
    if (FragPos.y > 0) {SS
        materialColor = materialColor4;
    }

    // Slope
    if (abs(Normal.x) > 0.35) {
        materialColor = materialColor3;
    }
    */

    // Fr�n vertex shader
    materialColor = vertColour;
        
    // Heights (resten i vertex shader, interpolerat ist�llet f�r rakt)
    // Ocean (over slope)
    if (FragPos.y <= 50) {
        materialColor = materialColor2;
    }

    vec3 ambient = lightAmbient * materialColor;
    vec3 diffuse = lightDiffuse * diff * materialColor;
    vec3 specular = lightSpecular * spec * materialColor;
 //   return vec3(specular);
    return (ambient + diffuse);
}

void main()
{    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = CalcDirLight(norm, viewDir);

    FragColor = vec4(vec3(result), 1.0f); // combining the two lighting components
}

