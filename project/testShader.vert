#version 420

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

out vec3 vertColour;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 materialColor1;
uniform vec3 materialColor2;
uniform vec3 materialColor3;
uniform vec3 materialColor4;
uniform vec3 materialColor5;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    TexCoords = aTexCoords;
    
    vertColour = materialColor2;
    // Heights ("ocean" i vertex shader, rakt istället för interpolerat)
    // Snow
    if (FragPos.y > 2400) {
        vertColour = materialColor5;
    } else 
    // Grass
    if (FragPos.y > 1750) {
        vertColour = materialColor1;
    } else
    // Sand
    if (FragPos.y > 1500) {
        vertColour = materialColor4;
    }

    // Slope
    if (abs(Normal.x) > 0.9) {
     //   vertColour = materialColor3;
    }
    gl_Position = projection * view * vec4(FragPos, 1.0);
}