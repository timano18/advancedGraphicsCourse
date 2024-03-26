#include <GL/glew.h>
#include <vector>
#include "perlinNoise.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "Grid.h" 

#include <iostream>

Grid::Grid()
{
	m_gridWidth = 120;
	m_gridHeight = 120;
	m_cellSize = 10.0f;
	m_noiseScale = 100.0f; // en slider i programmet?
}

// Parameterized constructor implementation
Grid::Grid(unsigned int gridWidth, unsigned int gridHeight, float cellSize, float noiseScale)
{
	m_gridWidth = gridWidth;
	m_gridHeight = gridHeight;
	m_cellSize = cellSize;
	m_noiseScale = noiseScale;
	// Initialize VAO, VBO, and EBO here or in generateGrid if appropriate
}

// generateGrid method implementation
void Grid::generateGrid()
{

	// Generate vertices
	for (int i = 0; i < m_gridHeight; i++) {
		for (int j = 0; j < m_gridWidth; j++) {
			//float z = perlinNoise(i, j, m_gridWidth, m_gridHeight, 750);		// perlin
			//float z = voronoiNoise(i, j, m_gridWidth, m_gridHeight, 750);	// voronoi
			float z = perlinNoise(i, j, m_gridWidth, m_gridHeight, 750) + voronoiNoise(i, j, m_gridWidth, m_gridHeight, 750); // both
			std::cout << z <<  '\n'; // test print
			vertices.push_back(j * m_cellSize);
			vertices.push_back(i * m_cellSize);
			vertices.push_back(z);
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
		}
	}


	

	

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
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

