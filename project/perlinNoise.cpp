#include <algorithm>
#include <cmath>
#include <vector>
#include <list>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#define pi 3.14159265

// ***** HELPER FUNCTIONS *****

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
	float random = a * (pi / ~(~0u >> 1)); // in [0, 2*Pi]

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

// Function to find closest neighbor in array("matrix")
float findClosest(std::vector<glm::vec2> posArr, int x, int y, bool closest) {

	glm::vec2 posIndex;

	std::list<float> distArr;

	float dx;
	float dy;
	float dist;

	for (int i = 0; i < posArr.size(); i++) {
		// Get coords. of element in list
		posIndex.x = posArr.at(i).x;
		posIndex.y = posArr.at(i).y;

		dx = abs(x - posIndex.x);
		dy = abs(y - posIndex.y);
		dist = pow(dx, 2) + pow(dy, 2); // distance without sqrt, faster

		distArr.push_back(dist);
	}

	distArr.sort();

	// take first or second element, "closest"
	if (closest) {
		return abs(distArr.front());
	}
	else {
		distArr.pop_front();
		return abs(distArr.front());
	}
}

//***** PERLIN START *****
// Input coords. to generate perlin in point
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
	float perlinValue = interpolate(ix0, ix1, wy);

	return perlinValue;
}

// Generate perlin map
float perlinNoise(float x, float y, int sizeX, int sizeY, float noiseScale) {

	int levels = 3;

	x = (float)x / (sizeX - 1);
	y = (float)y / (sizeY - 1);

	float normalizedP = 0.0;

	for (int i = levels; i > 0; i--) {
		normalizedP = normalizedP + (((perlinNoiseGen(x * pow(2, i), y * pow(2, i)) + 1.0) / 2.0) / i);
	}

	return (normalizedP / divHeight(levels)) * noiseScale;
}

//***** PERLIN END *****


//***** VORONOI START *****
// Input coords. to generate voronoi in point. OBS! KAN finnas en chans för 0 st. punkter? Ta bort genom "low size -> no calcs."
std::vector<glm::vec2> voronoiPointsGen(float x, float y, int sizeX, int sizeY, float randPts) {

	float voronoiValue;
	std::vector<glm::vec2> posArr;
	glm::vec2 pos;

	for (int j = 0; j < sizeY; ++j) {
		for (int i = 0; i < sizeX; ++i) {

			voronoiValue = abs(randomGradient(i + 1, j + 1).x); // +1 för att ta bort alltid prick (0,0)
			if (voronoiValue < (randPts / (sizeX * sizeY))) {

				pos.x = i;
				pos.y = j;

				posArr.push_back(pos); // Save coords. in array
			}
		}
	}
	return posArr;
}

// Generate voronoi map
float voronoiNoise(float x, float y, int sizeX, int sizeY, float noiseScale) {

	float randPts = 10; // 40: ok 240x240
	float reduceAmount = 5000; // 5000: good for 240x240
	float c1 =  -1.0;
	float c2 =  1.0;

	float voronoiValue;
	std::vector<glm::vec2> posArr = voronoiPointsGen(x, y, sizeX, sizeY, randPts);

	int nPts = posArr.size();

	if (nPts <= 2) {
		voronoiValue = 1.0; // To few points, return nothing
	}
	else {
		voronoiValue = c1 * findClosest(posArr, x, y, true) + c2 * findClosest(posArr, x, y, false);
	}

	voronoiValue = voronoiValue / reduceAmount; // Set to 0 - 1
	if (voronoiValue > 1.0) {
		voronoiValue = 1.0;
	}
	return voronoiValue * noiseScale;
}

//***** VORONOI END *****