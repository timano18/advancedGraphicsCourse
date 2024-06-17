#version 420

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aOffset;
//layout (location = 3) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

out vec3 vertColour;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float time;

float windAmpX = 2.5f*3;
float windAmpY = 1.65f*3;
float windFreqX = 6.0f;
float windFreqY = 2.5f;

void main() {
    //vec3 aPos = vec3(aPos.y, aPos.x, aPos.z);
    
    float totalWindOffsetX = cos(time * windFreqX - aOffset.x * 0.1) * windAmpX;
    float totalWindOffsetY = sin(time * windFreqY - aOffset.y * 0.1) * windAmpY;
    
    if (aTexCoords.y == 1.0f) { // "if top vertex" (from texCoord)
        FragPos = vec3(model * vec4(aPos.x + aOffset.x + totalWindOffsetX, aPos.y  + aOffset.y + totalWindOffsetY , aPos.z + aOffset.z - 100.0, 1.0));
    }
    else {                      // "if bot vertex" (from texCoord)
        FragPos = vec3(model * vec4(aPos.x + aOffset.x, aPos.y + aOffset.y, aPos.z + aOffset.z - 100.0, 1.0));
    }
    
    //Normal = mat3(transpose(inverse(model))) * aNormal; // inga normaler in just nu
    TexCoords = aTexCoords;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}