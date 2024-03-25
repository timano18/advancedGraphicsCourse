#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;

void main() {
    float intensity = texture(texture1, TexCoord).r;
    FragColor = vec4(intensity, intensity, intensity,intensity);
}
