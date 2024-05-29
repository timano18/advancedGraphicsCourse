#include <math.h>
#include <cmath>
#include <ctime>
#include <iostream>
#include <vector>
#include <list>

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

// Function to find closest neighbor in array("matrix")
float findClosest(std::vector<vec2float> posArr, int x, int y, bool closest) {

    vec2float posIndex;

    std::list<float> list;

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

        list.push_back(dist);
    }

    list.sort();

    // test, check list
    /*
    for(int list1 : list) {
        std::cout << list1 << ",  ";
    }
    std::cout << '\n';
    */

    // take first or second element, "closest"
    if (closest) {
        //std::cout << "VALUE1: " << list.front() << '\n';
        return abs(list.front());
    } else {
        list.pop_front();
        //std::cout << "VALUE2: " << list.front() << '\n';
        return abs(list.front());
    }
}

///// GENERATE VORONOI NOICE ///// END

int main() {

	// Image

	int image_width = 240;
	int image_height = 240;

    vec2float startPos;
    startPos.x = 0;
    startPos.y = 0;

    float randPts = 40; // 40 ok?

	// Render
	
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    // Parameters
    int colorValue;
    float voronoiValue = 1.0;
    std::vector<vec2float> posArr;

    // Values for terrain
    float c1 = -1.0;
    float c2 =  1.0;

    // For-loop creating ranom points. OBS! Extra kod här för att "måla ut". Behöver bara listan "posArr" egentligen
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

            voronoiValue = abs(randomGradient(i + 1 + startPos.x, j + 1 + startPos.y).x); // +1 för att ta bort alltid prick (0,0). OBS! i OCH j MÅSTE VARA KORDINATERNA! INTE 0 TILL 240 VARJE GÅNG!
            //std::cout << "Voronoi is: " << voronoiValue<< '\n';
           if (voronoiValue < randPts/(image_width*image_height)) {
                //voronoiValue = 0.0;

                vec2float pos;
                pos.x = i;
                pos.y = j;

                posArr.push_back(pos); // Save coords. in array
                //std::cout << "Point coordinates: " << posArr.back().x << "X and " << posArr.back().y << "Y" << '\n';
            //} else {
            //    voronoiValue = 1.0;
            //}

            //colorValue = static_cast<int>(255.999 * voronoiValue); // Om vi ska använda "full slump" måste vi ha ett filter vid en chunks kanter. OBS, verkar generera samma iaf?
            //std::cout << colorValue << ' ' << colorValue << ' ' << colorValue << '\n';
           }
        }
    }

    int nPts = posArr.size();

    // for-loop interpolating voronoi
	for (int j = 0; j < image_height; ++j) {
		for (int i = 0; i < image_width; ++i) {

            // Create function from paper (only use c1 and c2 for "mountains")
           

            if (nPts <= 2) {
                voronoiValue = 1.0; // To few points, return nothing
                //std::cout << voronoiValue << '\n';
            } else {
                voronoiValue = c1*findClosest(posArr, i, j, true) + c2*findClosest(posArr, i, j, false);
                // To few points, return nothing
                //std::cout << c1*findClosest(posArr, i, j, 1) << '\n';
                //std::cout << c2*findClosest(posArr, i, j, 2) << '\n';
                //std::cout << voronoiValue << '\n';
            }

            float reduceValue = 2500; // Between 0 and 1, testa sig fram? 240x240: 5000 ser bra ut!
            voronoiValue = voronoiValue/reduceValue;
            if (voronoiValue > 1.0) {
                voronoiValue = 1.0;
            }
            //std::cout << voronoiValue << '\n';

            colorValue = static_cast<int>(255.999 * voronoiValue); // Om vi ska använda "full slump" måste vi ha ett filter vid en chunks kanter. OBS, verkar generera samma iaf?

            std::cout << colorValue << ' ' << colorValue << ' ' << colorValue << '\n';
        }
    }

	std::clog << "\rDone.\n";
	
}

/*
            // Create function from paper (only use c1 and c2 for "mountains")
            int nPts = posArr.size();

            if (nPts <= 2) {
                voronoiValue = 1.0; // To few points, return nothing
                std::cout << voronoiValue << '\n';
            } else {
                voronoiValue = c1*findClosest(posArr, 0, 1) + c2*findClosest(posArr, 0, 2);
                std::cout << voronoiValue << '\n';
            }

		if (distArr.empty()) {
			distArr.push_back(dist);
		} else {
			int j = 0;
			while (dist < distArr.at(j) && j < distArr.size()) {
				distArr.push_front(dist)
				j++;
			}
		}


*/