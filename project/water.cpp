#include <iostream>
#include <GL/glew.h>

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "Water.h"

#include "Grid.h"

// Create water object
Water::Water(float startX, float startY, float stopX, float stopY, float zStartPos) {
	m_startX = startX;
	m_startY = startY;
	m_stopX = stopX; 
	m_stopY = stopY; 
	m_zStartPos = zStartPos;
}

void Water::genQuad() {

	// ...
}

// Draw water
void Water::drawWater() {

	// ...
}