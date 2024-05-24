#version 420

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float waterPlaneHeight;
uniform float waterPlaneDirection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    
    gl_ClipDistance[0] = dot(vec4(aPos, 1.0), vec4(0.0f, 0.0f, waterPlaneDirection, waterPlaneHeight));

    gl_Position = projection * view * vec4(FragPos, 1.0);
}