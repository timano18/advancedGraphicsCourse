#version 420
// This vertex shader simply outputs the input coordinates to the rasterizer. It only uses 2D coordinates.
layout(location = 0) in vec2 position;

out vec2 texCoord;
out vec3 pos;
uniform mat4 view;
uniform mat4 projection;

uniform float waterPlaneHeight;
uniform float waterPlaneDirection;

void main()
{
	gl_Position = vec4(position, 0.0, 1.0);
	gl_ClipDistance[0] = dot(vec4(position,0.0, 1.0), vec4(0.0f, 0.0f, waterPlaneDirection, waterPlaneHeight));
	texCoord = 0.5 * (position + vec2(1, 1));
	pos = transpose(mat3(view)) * (inverse(projection) * gl_Position).xyz;
}

