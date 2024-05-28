#include <GL/glew.h>
#include <vector>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "Grass.h" 

#include <iostream>

#define PI 3.14159265

// One grass
Grass::Grass()
{
	m_width = 5;
	m_height = 10;
	m_xPos = 0;
	m_yPos = 0;
	m_zPos = 0;
}

// Parameterized constructor implementation
Grass::Grass(unsigned int width, unsigned int height, int xPos, int yPos, int zPos)
{

	m_width = width;
	m_height = height;
	m_xPos = xPos; 
	m_yPos = yPos;
	m_zPos = -zPos;
}

// *** Generate grass ***
// generateGrid method implementation
void Grass::generateGrassSquare()
{

	float startX = m_xPos - 0.5 * m_width;
	float startY = m_yPos - 0.5 * m_width;

	float stopX = m_xPos + 0.5 * m_width;
	float stopY = m_yPos + 0.5 * m_width;

	float bot = m_zPos;
	float top = bot + m_height;


	// PLANE
	float grassSquare[] = {
	startX, m_yPos, bot,				0.0f, 1.0f, // BL 0
	stopX, m_yPos, bot,					1.0f, 1.0f, // BR 1
	startX, m_yPos, top,				0.0f, 0.0f, // TL 2

	stopX, m_yPos, bot,					1.0f, 1.0f, // BR 1
	stopX, m_yPos, top,					1.0f, 0.0f, // TR 3
	startX, m_yPos, top,				0.0f, 0.0f  // TL 2
	};

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);

	indices.push_back(3);
	indices.push_back(4);
	indices.push_back(5);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grassSquare), &grassSquare, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	// Pos
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	// TexCoords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));



	glBindVertexArray(0);
}

void Grass::generateGrassStar()
{

	float startX1 = m_xPos - 0.5 * m_width * cos(0 * (PI/180));
	float startY1 = m_yPos - 0.5 * m_width * sin(0 * (PI / 180));

	float stopX1 = m_xPos + 0.5 * m_width * cos(0 * (PI / 180));
	float stopY1 = m_yPos + 0.5 * m_width * sin(0 * (PI / 180));

	float startX2 = m_xPos - 0.5 * m_width * cos(60 * (PI / 180));
	float startY2 = m_yPos - 0.5 * m_width * sin(60 * (PI / 180));

	float stopX2 = m_xPos + 0.5 * m_width * cos(60 * (PI / 180));
	float stopY2 = m_yPos + 0.5 * m_width * sin(60 * (PI / 180));

	float startX3 = m_xPos - 0.5 * m_width * cos(-60 * (PI / 180));
	float startY3 = m_yPos - 0.5 * m_width * sin(-60 * (PI / 180));

	float stopX3 = m_xPos + 0.5 * m_width * cos(-60 * (PI / 180));
	float stopY3 = m_yPos + 0.5 * m_width * sin(-60 * (PI / 180));

	float bot = m_zPos;
	float top = bot + m_height;

	// PLANE
	float grassStar[] = {
	// 1
	startX1, startY1, bot,				0.0f, 1.0f, // BL 0
	stopX1, stopY1, bot,				1.0f, 1.0f, // BR 1
	startX1, startY1, top,				0.0f, 0.0f, // TL 2

	stopX1, startY1, bot,				1.0f, 1.0f, // BR 1
	stopX1, stopY1, top,				1.0f, 0.0f, // TR 3
	startX1, stopY1, top,				0.0f, 0.0f, // TL 2
	// 2
	startX2, startY2, bot,				0.0f, 1.0f, // BL 0
	stopX2, stopY2, bot,				1.0f, 1.0f, // BR 1
	startX2, startY2, top,				0.0f, 0.0f, // TL 2

	stopX2, stopY2, bot,				1.0f, 1.0f, // BR 1
	stopX2, stopY2, top,				1.0f, 0.0f, // TR 3
	startX2, startY2, top,				0.0f, 0.0f, // TL 2
	// 3
	startX3, stopY2, bot,				0.0f, 1.0f, // BL 0
	stopX3, startY2, bot,				1.0f, 1.0f, // BR 1
	startX3, stopY2, top,				0.0f, 0.0f, // TL 2

	stopX3, stopY3, bot,				1.0f, 1.0f, // BR 1
	stopX3, stopY3, top,				1.0f, 0.0f, // TR 3
	startX3, startY3, top,				0.0f, 0.0f, // TL 2
	};

	// 1
	indices.push_back(0);
	normals.push_back(1.0);
	indices.push_back(1);
	normals.push_back(1.0);
	indices.push_back(2);
	normals.push_back(1.0);

	indices.push_back(3);
	normals.push_back(1.0);
	indices.push_back(4);
	normals.push_back(1.0);
	indices.push_back(5);
	normals.push_back(1.0);

	// 2
	indices.push_back(6);
	normals.push_back(1.0);
	indices.push_back(7);
	normals.push_back(1.0);
	indices.push_back(8);
	normals.push_back(1.0);

	indices.push_back(9);
	normals.push_back(1.0);
	indices.push_back(10);
	normals.push_back(1.0);
	indices.push_back(11);
	normals.push_back(1.0);

	// 3
	indices.push_back(12);
	normals.push_back(1.0);
	indices.push_back(13);
	normals.push_back(1.0);
	indices.push_back(14);
	normals.push_back(1.0);

	indices.push_back(15);
	normals.push_back(1.0);
	indices.push_back(16);
	normals.push_back(1.0);
	indices.push_back(17);
	normals.push_back(1.0);
	

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grassStar), &grassStar, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void Grass::generateGrassTriangle()
{

	float startX1 = (m_xPos - 0.5 * m_width * cos(0 * (PI / 180)));
	float startY1 = (m_yPos - 0.5 * m_width * sin(0 * (PI / 180))) - 0.2 * m_width;

	float stopX1 = (m_xPos + 0.5 * m_width * cos(0 * (PI / 180)));
	float stopY1 = (m_yPos + 0.5 * m_width * sin(0 * (PI / 180))) - 0.2 * m_width;

	float startX2 = (m_xPos - 0.5 * m_width * cos(60 * (PI / 180))) - 0.1 * m_width;
	float startY2 = (m_yPos - 0.5 * m_width * sin(60 * (PI / 180))) ;

	float stopX2 = (m_xPos + 0.5 * m_width * cos(60 * (PI / 180))) - 0.1 * m_width;
	float stopY2 = (m_yPos + 0.5 * m_width * sin(60 * (PI / 180))) ;

	float startX3 = (m_xPos - 0.5 * m_width * cos(-60 * (PI / 180))) + 0.1 * m_width;
	float startY3 = (m_yPos - 0.5 * m_width * sin(-60 * (PI / 180))) ;

	float stopX3 = (m_xPos + 0.5 * m_width * cos(-60 * (PI / 180))) + 0.1 * m_width;
	float stopY3 = (m_yPos + 0.5 * m_width * sin(-60 * (PI / 180))) ;

	float bot = m_zPos;
	float top = bot + m_height;

	// PLANE
	float grassStar[] = {
		// 1
		startX1, startY1, bot,				0.0f, 1.0f, // BL 0
		stopX1, stopY1, bot,				1.0f, 1.0f, // BR 1
		startX1, startY1, top,				0.0f, 0.0f, // TL 2

		stopX1, startY1, bot,				1.0f, 1.0f, // BR 1
		stopX1, stopY1, top,				1.0f, 0.0f, // TR 3
		startX1, stopY1, top,				0.0f, 0.0f, // TL 2
		// 2
		startX2, startY2, bot,				0.0f, 1.0f, // BL 0
		stopX2, stopY2, bot,				1.0f, 1.0f, // BR 1
		startX2, startY2, top,				0.0f, 0.0f, // TL 2

		stopX2, stopY2, bot,				1.0f, 1.0f, // BR 1
		stopX2, stopY2, top,				1.0f, 0.0f, // TR 3
		startX2, startY2, top,				0.0f, 0.0f, // TL 2
		// 3
		startX3, stopY2, bot,				0.0f, 1.0f, // BL 0
		stopX3, startY2, bot,				1.0f, 1.0f, // BR 1
		startX3, stopY2, top,				0.0f, 0.0f, // TL 2

		stopX3, stopY3, bot,				1.0f, 1.0f, // BR 1
		stopX3, stopY3, top,				1.0f, 0.0f, // TR 3
		startX3, startY3, top,				0.0f, 0.0f, // TL 2
	};

	// 1
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);

	indices.push_back(3);
	indices.push_back(4);
	indices.push_back(5);

	// 2
	indices.push_back(6);
	indices.push_back(7);
	indices.push_back(8);

	indices.push_back(9);
	indices.push_back(10);
	indices.push_back(11);

	// 3
	indices.push_back(12);
	indices.push_back(13);
	indices.push_back(14);

	indices.push_back(15);
	indices.push_back(16);
	indices.push_back(17);


	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grassStar), &grassStar, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
}

// Draw grass
void Grass::DrawGrass()
{
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}