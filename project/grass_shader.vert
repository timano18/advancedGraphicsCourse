#version 420

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec2 aOffset;
//layout (location = 3) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

out vec3 vertColour;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float waterPlaneHeight;
uniform float waterPlaneDirection;

void main()
{
    //FragPos = vec3(model * vec4(aPos + (100.0, 100.0, 100.0), 1.0));
    //FragPos = vec3(model * vec4(aPos.x + 0.0, aPos.y + 0.0, aPos.z + 0.0, 1.0));
    FragPos = vec3(model * vec4(aPos.x + aOffset.x, aPos.y + aOffset.y, aPos.z + 0.0, 1.0));
    //Normal = mat3(transpose(inverse(model))) * aNormal; // inga normaler in just nu
    TexCoords = aTexCoords;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}