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
int divider = 1;
unsigned int s_width = 2400;
unsigned int s_height = 2400;
int s_xStartPos = -100;
int s_yStartPos = -100;
float s_cellSize = 1.0f;
float s_perlinScale = 750.0;
float s_voronoiScale = 100.0;

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
}

// Parameterized constructor implementation
Grid::Grid(unsigned int width, unsigned int height, int xStartPos, int yStartPos, float cellSize, float perlinScale, float voronoiScale)
{
	m_width = width;
	m_height = height;
	m_xStartPos = xStartPos; // - 1 for correct starting placements
	m_yStartPos = yStartPos; // - 1 for correct starting placements
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
	m_grids;
}

// Parameterized constructor implementation
GridChunk::GridChunk(int xStartPos_Chunk, int yStartPos_Chunk, int xEndPos_Chunk, int yEndPos_Chunk)
{
	m_xStartPos_Chunk = xStartPos_Chunk; 
	m_yStartPos_Chunk = yStartPos_Chunk;
	m_xEndPos_Chunk = xEndPos_Chunk;
	m_yEndPos_Chunk = yEndPos_Chunk;
	m_grids;
}

GLuint Grid::getVBO()
{
	return VBO;
}
// *** Generate grid ***
// generateGrid method implementation
void Grid::generateGrid()
{

	// Generate voronoiPoints before loop
	float randPts = 20;

	float randValue;
	std::vector<glm::vec2> posArr;
	glm::vec2 pos;

	for (int i = 0; i < m_width; ++i) {
		for (int j = 0; j < m_height; ++j) {

			randValue = abs(randomGradient(i + 1, j + 1).x); // +1 f�r att ta bort alltid prick (0,0)
			if (randValue < (randPts / (m_width * m_height))) {
	
				pos.x = i;
				pos.y = j;
	
				posArr.push_back(pos); // Save coords. in array
			}
		}
	}

	// Positions for the loops (start/stop coords.)
	int startX = m_xStartPos;
	int startY = m_yStartPos;
	int stopX  = m_xStartPos + m_width;
	int stopY  = m_yStartPos + m_height;

	// Generate vertices
	for (int i = startY; i < stopY; i++) {
		for (int j = startX; j < stopX; j++) {

			// Generate noise
			//float z = perlinNoise(i, j, m_width, m_height, m_perlinScale) + voronoiNoise(i, j, m_width, m_height, posArr, m_voronoiScale);
			//float z = perlinNoise(i * m_cellSize, j * m_cellSize, m_width, m_height, m_perlinScale );
			Vertex vertex;
			vertex.position = glm::vec3(i * m_cellSize, j * m_cellSize , 0);
			vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);  // Ensure the normal is initialized to zero
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
			indices.push_back(bottomRight);
			indices.push_back(bottomLeft);
			indices.push_back(topLeft);
		
			

			// Second triangle (top-left, bottom-right, top-right)
			indices.push_back(topRight);
			indices.push_back(bottomRight);
			indices.push_back(topLeft);
		
		

			/*
			
			// Calculate normals for the two triangles
			glm::vec3 normal1 = (glm::cross(
				vertices[bottomLeft].position - vertices[topLeft].position,
				vertices[bottomRight].position - vertices[topLeft].position));

			glm::vec3 normal2 = (glm::cross(
				vertices[bottomRight].position - vertices[topLeft].position,
				vertices[topRight].position - vertices[topLeft].position));

			// Add normals to the vertices
			
			vertices[topLeft].normal += (normal1 + normal2);
			vertices[bottomLeft].normal += (normal1);
			vertices[bottomRight].normal += (normal1 + normal2);
			vertices[topRight].normal += (normal2);
			*/
	
		}
	}
	/*
	for (int i = 0; i < vertices.size(); i++) {
		
	
			
		glm::vec3 off = glm::vec3(1.0, 1.0, 0.0);
		float hL = perlinNoise(vertices[i].position.x/1.0 - off.x, vertices[i].position.y / 1.0 - off.z, m_width, m_height, m_perlinScale);
		float hR = perlinNoise(vertices[i].position.x / 1.0 + off.x, vertices[i].position.y / 1.0 + off.z, m_width, m_height, m_perlinScale);
		float hD = perlinNoise(vertices[i].position.x / 1.0 - off.z, vertices[i].position.y / 1.0 - off.y, m_width, m_height, m_perlinScale);
		float hU = perlinNoise(vertices[i].position.x / 1.0 + off.z, vertices[i].position.y / 1.0 + off.y, m_width, m_height, m_perlinScale);

		vertices[i].normal.x = hL - hR;
		vertices[i].normal.y = hD - hU;
		vertices[i].normal.z = -20.0;
		//vertices[i].normal = glm::normalize(vertices[i].normal);
		
	}
	*/
	/*
	for (int i = 0; i < vertices.size(); i++) {
		vertices[i].normal = glm::normalize(vertices[i].normal);
	}
	*/
	/*
	for (int i = 0; i < vertices.size()/ 24; i++) {
		std::cout << "index: " << i << std::endl;
		std::cout << "x: " << vertices[i].normal.x << ", ";
		std::cout << "y: " << vertices[i].normal.y << ", ";
		std::cout << "z: " << vertices[i].normal.z << std::endl;
	}
	*/
	
	


	// *** Buffers ***
	//std::cout << "size of vertex list: " << vertices.size() << std::endl;
	//std::cout << "size of vertex: " << vertices.data() << std::endl;
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

// *** Generate gridChunk ***
std::vector<Grid> GridChunk::generateChunkGrids(int xStartPosChunk, int yStartPosChunk, int xEndPosChunk, int yEndPosChunk, unsigned int gridWidth, unsigned int gridHeight, float cellSize, float perlinScale, float voronoiScale) {

	std::vector<Grid> grids;

	// M�ste g�ra s� att t.ex. pos (x: 1, y: -5) fungerar, d.v.s. negativa koordinater
	// "-1" fungerar bara om tv� n�rligande grids har samma "uppl�sning", s� den hoppar med en "cellSize". F�r fixa

	for (int i = xStartPosChunk; i < xEndPosChunk; i++) {
		for (int j = yStartPosChunk; j < yEndPosChunk; j++) {
			Grid grid(gridWidth, gridHeight, (gridWidth - 1) * i, (gridHeight - 1) * j, cellSize, perlinScale, voronoiScale);
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

void GridChunk::createNewStandardChunk(int startX, int startY, int endX, int endY) {
	m_xStartPos_Chunk = startX;
	m_yStartPos_Chunk = startY;
	m_xEndPos_Chunk = endX;
	m_yEndPos_Chunk = endY;
	m_grids = generateChunkGrids(startX, startY, endX, endY, s_width, s_height, s_cellSize, s_perlinScale, s_voronoiScale);
};

void GridChunk::createNewChunk(int startX, int startY, int endX, int endY, unsigned int gridWidth, unsigned int gridHeight, float cellSize, float perlinScale, float voronoiScale) {
	m_xStartPos_Chunk = startX;
	m_yStartPos_Chunk = startY;
	m_xEndPos_Chunk = endX;
	m_yEndPos_Chunk = endY;
	m_grids = generateChunkGrids(startX, startY, endX, endY, gridWidth, gridHeight, cellSize, perlinScale, voronoiScale);
};

// GridChunk draw
void drawChunkGrid(Grid grid) { grid.DrawGrid(); } // Draw gridChunk: Function to draw grid-array, declare before main (grid.Draw() on "for_each")
void GridChunk::DrawGridChunk() {
	std::for_each(m_grids.begin(), m_grids.end(), drawChunkGrid);
	//std::cout << "Number of grids in chunk: " << m_grids.size() << '\n'; // F�r fels�kning, kan ta bort
};

// Find gridChunk center
glm::vec3 GridChunk::gridChunkCenter()
{
	glm::vec3 coordinates;

	coordinates = glm::vec3(0,0,0); // Ej klar!

	// Put camera at the center of created grid(s)				// Ej f�tt detta att fungera �nnu, kameran b�rjar ej p� (0,0). F�r s�tta v�rldens origo till kanten p� f�rsta chunken (eller i mitten, d�r kameran b�rjar)
	// orginelt: vec3 cameraPosition(186.0f, 829.0f, 1080.0f);
	//cameraPosition.x = (startGridsX * gridWidth * cellSize) / 2 - (0.5 * gridWidth * cellSize);
	//cameraPosition.y = (startGridsY * gridHeight * cellSize) / 2;
	//cameraPosition.z = 1000;

	return coordinates;
};
