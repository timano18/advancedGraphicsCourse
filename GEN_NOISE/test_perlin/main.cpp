#include <math.h>
#include <cmath>
#include <iostream>
///// GENERATE PERLIN NOICE ///// START


// vec2 with floats
typedef struct {
    float x, y;
} vec2float;

// Generate randomness (paste)
vec2float randomGradient(int ix, int iy) {
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

// Input coords.
float perlinNoice(float x, float y) {

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
    float value = interpolate(ix0, ix1, wy);

    return value;
}

// Calculated value to divide hight by st avoid values of > 1
float divHeight(int levels) {
    float divValue = 1;
    for (int i = 1; i < levels; i++) {
        divValue = divValue + (1/pow(2, i));
    }
    //std::cout << "DIVVALUE: " << divValue << '\n';
    return divValue;
}

///// GENERATE PERLIN NOICE ///// END

int main() {

	// Image

	int image_width = 512;
	int image_height = 512;

    int levels = 12;

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

            float normalizedP = 0.0;
            
            for (int i = levels; i > 0; i--) {
                normalizedP = normalizedP + (((perlinNoice(x*pow(2, i), y*pow(2, i)) + 1.0) / 2.0) / i);
            }
            
            // Map Perlin noise output to 0-255 range
            int colorValue = static_cast<int>(255.999 * (normalizedP/divHeight(levels))); // +0.35 i nämnaren vid "levels = 24", kan bli för små värden vid stora värden. OCKSÅ! ta bort "toFloatVec2"

            std::cout << colorValue << ' ' << colorValue << ' ' << colorValue << '\n';
        
		}
	}

	std::clog << "\rDone.				\n";
	
}