#include <GL/glew.h>
#include <vector>
#include "noise.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "Grid.h" 

#include <iostream>

Grid::Grid()
{
	m_gridWidth = 240;
	m_gridHeight = 240;
	m_xStartPos = 0;
	m_yStartPos = 0;
	m_cellSize = 10.0f;
	m_perlinScale = 750.0f;
	m_voronoiScale = 100.0f;
}

// Parameterized constructor implementation
Grid::Grid(unsigned int gridWidth, unsigned int gridHeight, int xStartPos, int yStartPos, float cellSize, float perlinScale, float voronoiScale)
{
	m_gridWidth = gridWidth;
	m_gridHeight = gridHeight;
	m_xStartPos = xStartPos; // - 1 for correct starting placements
	m_yStartPos = yStartPos;
	m_cellSize = cellSize;
	m_perlinScale = perlinScale;
	m_voronoiScale = voronoiScale;
	// Initialize VAO, VBO, and EBO here or in generateGrid if appropriate
}


// generateGrid method implementation
void Grid::generateGrid()
{

	// Generate voronoiPoints before loop
	float randPts = 20;

	float randValue;
	std::vector<glm::vec2> posArr;
	glm::vec2 pos;

	for (int i = 0; i < m_gridWidth; ++i) {
		for (int j = 0; j < m_gridHeight; ++j) {

			randValue = abs(randomGradient(i + 1, j + 1).x); // +1 för att ta bort alltid prick (0,0)
			if (randValue < (randPts / (m_gridWidth * m_gridHeight))) {
	
				pos.x = i;
				pos.y = j;
	
				posArr.push_back(pos); // Save coords. in array
			}
		}
	}

	// Positions for the loops (start/stop coords.)
	int startX = m_xStartPos;
	int startY = m_yStartPos;
	int stopX  = m_xStartPos + m_gridWidth;
	int stopY  = m_yStartPos + m_gridHeight;

	// Generate vertices
	for (int i = startY; i < stopY; i++) {
		for (int j = startX; j < stopX; j++) {

			// Generate noise
			float z = perlinNoise(i, j, m_gridWidth, m_gridHeight, m_perlinScale) + voronoiNoise(i, j, m_gridWidth, m_gridHeight, posArr, m_voronoiScale);

			Vertex vertex;
			vertex.position = glm::vec3(j * m_cellSize, i * m_cellSize, z);
			vertices.push_back(vertex);

		}
	}

	// Generate indices
	for (int i = 0; i < m_gridHeight - 1; i++) {
		for (int j = 0; j < m_gridWidth - 1; j++) {
			unsigned int topLeft = i * m_gridWidth + j;
			unsigned int topRight = topLeft + 1;
			unsigned int bottomLeft = (i + 1) * m_gridWidth + j;
			unsigned int bottomRight = bottomLeft + 1;

			// First triangle (top-left, bottom-left, bottom-right)
			indices.push_back(topLeft);
			indices.push_back(bottomLeft);
			indices.push_back(bottomRight);

			// Second triangle (top-left, bottom-right, top-right)
			indices.push_back(topLeft);
			indices.push_back(bottomRight);
			indices.push_back(topRight);

			// Calculate normals for the two triangles
			glm::vec3 normal1 = glm::normalize(glm::cross(
				vertices[bottomLeft].position - vertices[topLeft].position,
				vertices[bottomRight].position - vertices[topLeft].position));

			glm::vec3 normal2 = glm::normalize(glm::cross(
				vertices[bottomRight].position - vertices[topLeft].position,
				vertices[topRight].position - vertices[topLeft].position));

			// Add normals to the vertices
			vertices[topLeft].normal += normal1 + normal2;
			vertices[bottomLeft].normal += normal1;
			vertices[bottomRight].normal += normal1 + normal2;
			vertices[topRight].normal += normal2;
		}
	}

	for (int i = 0; i < vertices.size(); i++) {
		vertices[i].normal = glm::normalize(vertices[i].normal);
	}



	

	

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

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



// Draw method implementation
void Grid::Draw()
{
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

void Grid::setGridWidth(unsigned int gridWidth)
{
	m_gridWidth = gridWidth;
}
void Grid::setGridHeight(unsigned int gridHeight)
{
	m_gridHeight = gridHeight;
}

