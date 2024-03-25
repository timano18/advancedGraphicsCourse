#version 420
///////////////////////////////////////////////////////////////////////////////
// Input vertex attributes
///////////////////////////////////////////////////////////////////////////////
layout(location = 0) in vec3 position;

layout(location = 1) in vec3 normalIn;
layout(location = 2) in vec2 texCoordIn;

///////////////////////////////////////////////////////////////////////////////
// Input uniform variables
///////////////////////////////////////////////////////////////////////////////
uniform mat4 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;


uniform float currentTime; // Uniform variable to control animation over time
///////////////////////////////////////////////////////////////////////////////
// Output to fragment shader
///////////////////////////////////////////////////////////////////////////////
out vec2 texCoord;
out vec3 viewSpaceNormal;
out vec3 viewSpacePosition;


void main()
{
	float amplitude = 50.0;
	float frequency = 0.01; // Controls the frequency. Higher values = more waves.
	float speed = 1; // Controls the speed of the wave movement.

	// Calculate the zValue with a consistent period, adding time to the phase shift
	float zValue = amplitude * sin(frequency * position.x + currentTime * speed);  
	gl_Position = modelViewProjectionMatrix * vec4(position.x, position.y, zValue, 1.0);
	texCoord = texCoordIn;
	viewSpaceNormal = (normalMatrix * vec4(normalIn, 0.0)).xyz;
	viewSpacePosition = (modelViewMatrix * vec4(position, 1.0)).xyz;

}
