#include <GL/glew.h>
#include <vector>
#include "perlinNoise.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "Grid.h" 

Grid::Grid()
{
	m_gridWidth = 100;
	m_gridHeight = 100;
	m_cellSize = 10.0f;
	m_noiseScale = 2.0f;
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
			float z = 1000 * perlinNoice(i, j, m_gridWidth, m_gridHeight, m_noiseScale);
			Vertex vertex;
			vertex.position = glm::vec3(j * m_cellSize, i * m_cellSize, z);
			vertices.push_back(vertex);
			/*
			vertices.push_back(j * m_cellSize);
			vertices.push_back(i * m_cellSize);
			vertices.push_back(z);
			*/
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

