#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

glm::vec2 randomGradient(int ix, int iy);

float dotGridGradient(int ix, int iy, float x, float y);

float interpolate(float a0, float a1, float weight);

float divHeight(int levels);

float findClosest(std::vector<glm::vec2> posArr, int x, int y, bool closest);


float perlinNoise(float x, float y, int sizeX, int sizeY, float noiseScale);

float perlinNoiseGen(float x, float y);


float voronoiNoise(float x, float y, int sizeX, int sizeY, std::vector<glm::vec2> posArr, float noiseScale);