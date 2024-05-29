#version 420

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

out vec3 vertColour;
out vec4 vertexPosition; 
out float visibility;
out vec4 clipSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;



uniform float waterPlaneHeight;
uniform float waterPlaneDirection;


const float density = 0.00006;
const float gradient = 3.5;

void main()
{

    vertexPosition = view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    TexCoords = aTexCoords;

    gl_ClipDistance[0] = dot(vec4(aPos, 1.0), vec4(0.0f, 0.0f, waterPlaneDirection, waterPlaneHeight));
    
    // Heights ("ocean" i vertex shader, rakt istället för interpolerat)
    // Snow

    float distance = length(vertexPosition.xyz);
    visibility = exp(-pow((distance*density), gradient));
    visibility = clamp(visibility,0.0,1.0);

    gl_Position = projection * view * vec4(FragPos, 1.0);
    clipSpace = gl_Position;

}