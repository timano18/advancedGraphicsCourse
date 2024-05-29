#include <math.h>
#include <cmath>
#include <ctime>
#include <iostream>

#define pi 3.14159265

///// GENERATE VORONOI NOICE ///// START

// vec2 with floats
typedef struct {
	float x, y;
} vec2float;

// Generate randomness (paste)
vec2float randomGradient(int ix, int iy)
{
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
	vec2float v;
	v.x = sin(random);
	v.y = cos(random);

	return v;
}

// Find dot product of vectors
float dotGridGradient(int ix, int iy, float x, float y) {
    // Create random grad. for a square index
    vec2float gradient = randomGradient(ix, iy);

    // Find distance
    float dx = x - (float)ix;
    float dy = y - (float)iy;

    return (dx * gradient.x + dy * gradient.y);
}

// Interpolate two values with weight
float interpolate(float a0, float a1, float weight) {
    //return (a1 - a0) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a0;
    return (a1 - a0) * (3.0 - weight * 2.0) * weight * weight + a0;
}

///// GENERATE VORONOI NOICE ///// END

int main() {

	// Image

	int image_width = 240;
	int image_height = 240;

    float randPts = 30;

	// Render
	
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

	for (int j = 0; j < image_height; ++j) {
		for (int i = 0; i < image_width; ++i) {

            auto r = double(j) / (image_width-1);
			auto g = double(i) / (image_height-1);
			auto b = 0;

			int ir = static_cast<int>(255.999 * r);
			int ig = static_cast<int>(255.999 * g);
			int ib = static_cast<int>(255.999 * b);

            float x = (float)i / (image_width - 1);
            float y = (float)j / (image_height - 1);

            float voronoiValue;

            voronoiValue = abs(randomGradient(i, j).x);
            if (voronoiValue < randPts/(image_width*image_height)) {
                voronoiValue = 0.0;
            } else {
                voronoiValue = 1.0;
            }
            

            // Full rand.
            //voronoiValue = ((double) rand() / (RAND_MAX));

            int colorValue = static_cast<int>(255.999 * voronoiValue); // Om vi ska använda "full slump" måste vi ha ett filter vid en chunks kanter. OBS, verkar generera samma iaf?

            std::cout << colorValue << ' ' << colorValue << ' ' << colorValue << '\n';

        }
    }

	std::clog << "\rDone.\n";
	
}