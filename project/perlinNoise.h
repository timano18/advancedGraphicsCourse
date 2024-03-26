#pragma once

typedef struct {
	float x, y;
} vec2float;

vec2float randomGradient(int ix, int iy);

float dotGridGradient(int ix, int iy, float x, float y);

float interpolate(float a0, float a1, float weight);

float perlinNoice(float x, float y, int sizeX, int sizeY, float noiseScale);