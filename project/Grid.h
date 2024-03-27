#pragma once

#include <GL/glew.h>
#include <vector>

class Grid {
public: 
	Grid();
	Grid(unsigned int gridWidth, unsigned int gridHeight, float cellSize, float noiseScale);
	void generateGrid();
	void Draw();
	void setGridWidth(unsigned int gridWidth);
	void setGridHeight(unsigned int gridHeight);
private: 
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
	};
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<float> normals;
	unsigned int m_gridWidth;
	unsigned int m_gridHeight;
	float m_cellSize;
	float m_noiseScale;
	GLuint VAO, VBO, EBO;

};