#include <vector>
#include <algorithm>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

// Generate randomness
glm::vec2 randomGradient(int ix, int iy) {
	// No precomputed gradients mean this works for any number of grid coordinates
	const unsigned w = 8 * sizeof(unsigned);
	const unsigned s = w / 2;
	unsigned a = ix, b = iy;
	a *= 3284157443;

	b ^= a << s | a >> w - s;
	b *= 1911520717;

	a ^= b << s | b >> w - s;
	a *= 2048419325;
	float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]

	// Create the vector from the angle
	glm::vec2 v;
	v.x = sin(random);
	v.y = cos(random);

	return v;
}

// Find dot product of vectors
float dotGridGradient(int ix, int iy, float x, float y)
{
	// Create random grad. for a square index
	glm::vec2 gradient = randomGradient(ix, iy);

	// Find distance
	float dx = x - (float)ix;
	float dy = y - (float)iy;

	return (dx * gradient.x + dy * gradient.y);
}

// Interpolate two values with weight
float interpolate(float a0, float a1, float weight)
{
	return (a1 - a0) * (3.0 - weight * 2.0) * weight * weight + a0;
}

// Calculated value to divide hight by st avoid values of > 1
float divHeight(int levels) {
	float divValue = 1;
	for (int i = 1; i < levels; i++) {
		divValue = divValue + (1 / pow(2, i));
	}
	return divValue;
}

// Input coords.
float perlinNoiseGen(float x, float y) {

	// Find square corners
	int x0 = (int)x;
	int y0 = (int)y;
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	// Interpolation weight (from size of square)
	float wx = x - (float)x0;
	float wy = y - (float)y0;

	// Generate + interpolate top corners
	float n0 = dotGridGradient(x0, y0, x, y);
	float n1 = dotGridGradient(x1, y0, x, y);
	float ix0 = interpolate(n0, n1, wx);

	// Generate + interpolate bot corners
	n0 = dotGridGradient(x0, y1, x, y);
	n1 = dotGridGradient(x1, y1, x, y);
	float ix1 = interpolate(n0, n1, wx);

	// Interpolate between top and bot
	float genValue = interpolate(ix0, ix1, wy);

	return genValue;
}


// Input coords.
float perlinNoise(float x, float y, int sizeX, int sizeY, int levels, float noiseScale) {

	x = (float)x * noiseScale / (sizeX - 1);
	y = (float)y * noiseScale / (sizeY - 1);

	float normalizedP = 0.0;

	for (int i = levels; i > 0; i--) {
		normalizedP = normalizedP + (((perlinNoiseGen(x * pow(2, i), y * pow(2, i)) + 1.0) / 2.0) / i);
	}

	return (normalizedP / divHeight(levels)) * noiseScale;
}