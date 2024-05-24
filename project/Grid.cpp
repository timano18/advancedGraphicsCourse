#include <GL/glew.h>
#include <vector>
#include <algorithm>
#include "noise.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "Grid.h" 

#include <iostream>

// *** Parameters for standard grid and chunk ***
// For chunk
unsigned int s_width_Chunk = 3;
unsigned int s_height_Chunk = 3;

// For grid
unsigned int s_width = 240;
unsigned int s_height = 240;
int s_xStartPos = 0;
int s_yStartPos = 0;
float s_zStartPos = 0;
float s_cellSize = 10.0f;
float s_perlinScale = 1500.0; //750.0;
float s_voronoiScale = 200.0;

// Save positions for voronoi
std::vector<glm::vec2> posArrGlob;

// One grid
Grid::Grid()
{
	m_width = s_width;
	m_height = s_height;
	m_xStartPos = s_xStartPos;
	m_yStartPos = s_yStartPos;
	m_cellSize = s_cellSize;
	m_perlinScale = s_perlinScale;
	m_voronoiScale = s_voronoiScale;
	m_zStartPos = s_zStartPos;
}

// Parameterized constructor implementation
Grid::Grid(unsigned int width, unsigned int height, int xStartPos, int yStartPos, float zStartPos, float cellSize, float perlinScale, float voronoiScale)
{
	m_width = width;
	m_height = height;
	m_xStartPos = xStartPos; // - 1 for correct starting placements
	m_yStartPos = yStartPos; // - 1 for correct starting placements
	m_zStartPos = zStartPos;
	m_cellSize = cellSize;
	m_perlinScale = perlinScale;
	m_voronoiScale = voronoiScale;
	// Initialize VAO, VBO, and EBO here or in generateGrid if appropriate
}

// Group of grids
GridChunk::GridChunk()
{
	m_xStartPos_Chunk = 0;
	m_yStartPos_Chunk = 0;
	m_xEndPos_Chunk = s_width_Chunk;
	m_yEndPos_Chunk = s_height_Chunk;
	m_zStartPos = 0;
	m_grids;
}

// Parameterized constructor implementation
GridChunk::GridChunk(int xStartPos_Chunk, int yStartPos_Chunk, int xEndPos_Chunk, int yEndPos_Chunk, float height)
{
	m_xStartPos_Chunk = xStartPos_Chunk; 
	m_yStartPos_Chunk = yStartPos_Chunk;
	m_xEndPos_Chunk = xEndPos_Chunk;
	m_yEndPos_Chunk = yEndPos_Chunk;
	m_zStartPos = height;
	m_grids;
}


// *** Generate grid ***
// generateGrid method implementation
void Grid::generateGrid()
{
	float numTextures = 16.0f;

	// Positions for the loops (start/stop coords.)
	int startX = m_xStartPos;
	int startY = m_yStartPos;
	int stopX = m_xStartPos + m_width;
	int stopY = m_yStartPos + m_height;

	// Generate voronoiPoints before loop
	float borderValue = 0.6;
	posArrGlob = {};

	voronoiPoints(startX - m_width * borderValue, startY - m_height * borderValue, stopX + m_width * borderValue, stopY + m_height * borderValue);

	// Generate vertices
	for (int i = startY; i < stopY; i++) {
		for (int j = startX; j < stopX; j++) {

			// Perturbation
			glm::vec2 point = { i, j };
			point = perturbedNoice(point);

			// Generate noise
			float z = -m_zStartPos + perlinNoise(point.x, point.y, m_width, m_height, m_perlinScale) + voronoiNoise(point.x, point.y, m_width, m_height, posArrGlob, m_voronoiScale);
			Vertex vertex;
			vertex.position = glm::vec3(j * m_cellSize, i * m_cellSize, z);
			vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);  // Ensure the normal is initialized to zero
			vertex.texCoords = glm::vec3((float)j/ (float)(m_height / numTextures), (float)i/ (float)(m_width / numTextures), 0.0f);
			//std::cout << (float)j / (m_height / numTextures) << " & " << (float)i / (float)(m_width / numTextures) << '\n';
			vertices.push_back(vertex);

		}
	}

	// Generate indices
	for (int i = 0; i < m_height - 1; i++) {
		for (int j = 0; j < m_width - 1; j++) {
			unsigned int topLeft = i * m_width + j;
			unsigned int topRight = topLeft + 1;
			unsigned int bottomLeft = (i + 1) * m_width + j;
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
			vertices[topLeft].normal += glm::normalize(normal1 + normal2);
			vertices[bottomLeft].normal += glm::normalize(normal1);
			vertices[bottomRight].normal += glm::normalize(normal1 + normal2);
			vertices[topRight].normal += glm::normalize(normal2);
		}
	}

	for (int i = 0; i < vertices.size(); i++) {
		vertices[i].normal = glm::normalize(vertices[i].normal);
	}

	// *** Buffers ***
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

	// TexCoords attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoords)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

// *** Points for voronoi ***
void Grid::voronoiPoints(int startX, int startY, int stopX, int stopY) {

	float randPts = 10;
	float randValue;
	glm::vec2 pos;

	for (int i = startY; i < stopY; ++i) {
		for (int j = startX; j < stopX; ++j) {

			randValue = abs(randomGradient(i, j).x); // +1 för att ta bort alltid prick (0,0)
			if (randValue < (randPts / (m_width * m_height))) {

				pos.x = i;
				pos.y = j;
				//std::cout << "x: " << i << " & y: " << j << '\n';

				posArrGlob.push_back(pos); // Tillfällig
			}
		}
	}
}

/*
// *** Points for voronoi ***
void Grid::voronoiPoints(int startX, int startY) {
	float randPts = 10;

	float randValue;
	//std::vector<glm::vec2> posArr;
	glm::vec2 pos;

	int stopX = startX + m_width;
	int stopY = startY + m_height;

	for (int i = startY; i < stopY; ++i) {
		for (int j = startX; j < stopX; ++j) {

			randValue = abs(randomGradient(i+1, j+1).x); // +1 för att ta bort alltid prick (0,0)
			if (randValue < (randPts / (m_width * m_height))) {

				pos.x = i;
				pos.y = j;
				std::cout << "x: " << i << " & y: " << j << '\n';

				//posArr.push_back(pos); // Save coords. in array
				posArrGlob.push_back(pos); // Tillfällig
			}
		}
	}
}
*/


// *** Generate gridChunk ***
std::vector<Grid> GridChunk::generateChunkGrids(int xStartPosChunk, int yStartPosChunk, int xEndPosChunk, int yEndPosChunk, unsigned int gridWidth, unsigned int gridHeight, float cellSize, float zStartPos, float perlinScale, float voronoiScale) {

	std::vector<Grid> grids;

	// Måste göra så att t.ex. pos (x: 1, y: -5) fungerar, d.v.s. negativa koordinater
	// "-1" fungerar bara om två närligande grids har samma "upplösning", så den hoppar med en "cellSize". Får fixa
	
	for (int i = xStartPosChunk; i < xEndPosChunk; i++) {
		for (int j = yStartPosChunk; j < yEndPosChunk; j++) {
			//std::cout << '\n' << "Grid x: " << i << ", y: " << j << '\n' << "From x: " << (gridWidth - 1) * i << " to " << (gridWidth - 1) * i + gridWidth << '\n' << "From y: " << (gridHeight - 1) * j << " to " << (gridHeight - 1) * j + gridHeight << '\n';
			Grid grid(gridWidth, gridHeight, (gridWidth - 1) * i, (gridHeight - 1) * j, zStartPos, cellSize, perlinScale, voronoiScale);
			grid.generateGrid();
			grids.push_back(grid);
		}
	}

	return grids;
}


// *** Functions grid ***
// Draw grid
void Grid::DrawGrid()
{
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

// Change grid size
void Grid::setGridWidth(unsigned int width)
{
	m_width = width;
}
void Grid::setGridHeight(unsigned int height)
{
	m_height = height;
}


// *** Functions chunk ***

void GridChunk::createNewStandardChunk(int startX, int startY, int endX, int endY, float startZ) {
	m_xStartPos_Chunk = startX;
	m_yStartPos_Chunk = startY;
	m_xEndPos_Chunk = endX;
	m_yEndPos_Chunk = endY;
	m_grids = generateChunkGrids(startX, startY, endX, endY, s_width, s_height, s_cellSize, startZ, s_perlinScale, s_voronoiScale);
};

void GridChunk::createNewChunk(int startX, int startY, int endX, int endY, unsigned int gridWidth, unsigned int gridHeight, float cellSize, float startZ, float perlinScale, float voronoiScale) {
	m_xStartPos_Chunk = startX;
	m_yStartPos_Chunk = startY;
	m_xEndPos_Chunk = endX;
	m_yEndPos_Chunk = endY;
	m_grids = generateChunkGrids(startX, startY, endX, endY, gridWidth, gridHeight, cellSize, startZ, perlinScale, voronoiScale);
};

// GridChunk draw
void drawChunkGrid(Grid grid) { grid.DrawGrid(); } // Draw gridChunk: Function to draw grid-array, declare before main (grid.Draw() on "for_each")
void GridChunk::DrawGridChunk() {
	std::for_each(m_grids.begin(), m_grids.end(), drawChunkGrid);
	//std::cout << "Number of grids in chunk: " << m_grids.size() << '\n'; // För felsökning, kan ta bort
};

// Find gridChunk center
glm::vec3 GridChunk::gridChunkCenter()
{
	glm::vec3 coordinates;

	coordinates = glm::vec3(0,0,0); // Ej klar!

	// Put camera at the center of created grid(s)				// Ej fått detta att fungera ännu, kameran börjar ej på (0,0). Får sätta världens origo till kanten på första chunken (eller i mitten, där kameran börjar)
	// orginelt: vec3 cameraPosition(186.0f, 829.0f, 1080.0f);
	//cameraPosition.x = (startGridsX * gridWidth * cellSize) / 2 - (0.5 * gridWidth * cellSize);
	//cameraPosition.y = (startGridsY * gridHeight * cellSize) / 2;
	//cameraPosition.z = 1000;

	return coordinates;
};
