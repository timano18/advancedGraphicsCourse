#include <GL/glew.h>
#include <vector>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "Grass.h" 

#include <iostream>

// One grass
Grass::Grass()
{
	m_width = 5;
	m_height = 10;
	m_xPos = 0;
	m_yPos = 0;
}

// Parameterized constructor implementation
Grass::Grass(unsigned int width, unsigned int height, int xPos, int yPos)
{

	m_width = width;
	m_height = height;
	m_xPos = xPos; 
	m_yPos = yPos; 
}

// *** Generate grid ***
// generateGrid method implementation
void Grass::generateGrass()
{

	float startX = m_xPos;
	float startY = m_yPos;
	float stopX = m_xPos + m_width;
	float stopY = m_yPos + m_height;

	// PLANE
	/*float grassSquare[] = {
	stopX, stopY, 0.0f,				1.0f, 1.0f, // Top Right
	startX, stopY, 0.0f,			0.0f, 1.0f, // Top Left
	stopX, startY, 0.0f,			1.0f, 0.0f, // Bot Right

	startX, stopY, 0.0f,			0.0f, 1.0f, // Top Left
	startX, startY, 0.0f,			0.0f, 0.0f, // Bot Left
	stopX, startY, 0.0f,			1.0f, 0.0f  // Bot Right
	};*/

	float grassSquare[] = {
	1000.0f, 1000.0f, 1000.0f,			1.0f, 1.0f, // Top Right
	0.0f, 1000.0f, 1000.0f,			0.0f, 1.0f, // Top Left
	1000.0f, 0.0f, 1000.0f,			1.0f, 0.0f, // Bot Right

	0.0f, 1000.0f, 1000.0f,			0.0f, 1.0f, // Top Left
	0.0f, 0.0f, 1000.0f,				0.0f, 0.0f, // Bot Left
	1000.0f, 0.0f, 1000.0f,			1.0f, 0.0f  // Bot Right
	};

	// First triangle (top-left, bottom-left, bottom-right)
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);

	// Second triangle (top-left, bottom-right, top-right)
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(1);

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

	// Postition attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	// Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

// Draw grass (one)
void Grass::DrawGrass()
{
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	//glDrawArrays(GL_TRIANGLES, 0, 5);
}