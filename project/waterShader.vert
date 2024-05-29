#version 420

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 FresnelCamera;
out vec4 clipSpace;
out float visibility;
out vec4 vertexPosition; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;

const float density = 0.00006;
const float gradient = 3.5;

void main() {


    vertexPosition = view * model * vec4(aPos, 1.0);

    FragPos = vec3(model * vec4(aPos, 1.0));
    TexCoords = aTexCoords;
    FresnelCamera = viewPos - FragPos;
    clipSpace = projection * view * vec4(FragPos, 1.0);
    gl_Position = clipSpace;
    
    float distance = length(vertexPosition.xyz);
    visibility = exp(-pow((distance*density), gradient));
    visibility = clamp(visibility,0.0,1.0);
}