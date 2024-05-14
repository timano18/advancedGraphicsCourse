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

	Vertex vertex;

	// 0: topLeft
	vertex.position = glm::vec3(m_startX, m_stopY, m_zStartPos);
	vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);
	vertices.push_back(vertex);
	// 1: topRight
	vertex.position = glm::vec3(m_stopX, m_stopY, m_zStartPos);
	vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);
	vertices.push_back(vertex);
	// 2: bottomLeft
	vertex.position = glm::vec3(m_startX, m_startY, m_zStartPos);
	vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);
	vertices.push_back(vertex);
	// 3: bottomRight
	vertex.position = glm::vec3(m_stopX, m_startY, m_zStartPos);
	vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);
	vertices.push_back(vertex);

	// First triangle
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(3);
	// Second triangle
	indices.push_back(3);
	indices.push_back(0);
	indices.push_back(1);

	// Calculate normals for the two triangles
	glm::vec3 normal1 = glm::normalize(glm::cross(
		vertices[3].position - vertices[0].position,
		vertices[2].position - vertices[0].position));

	glm::vec3 normal2 = glm::normalize(glm::cross(
		vertices[1].position - vertices[0].position,
		vertices[3].position - vertices[0].position));

	// Add normals to the vertices
	vertices[0].normal += glm::normalize(normal1 + normal2);
	vertices[2].normal += glm::normalize(normal1);
	vertices[3].normal += glm::normalize(normal1 + normal2);
	vertices[1].normal += glm::normalize(normal2);

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

	glBindVertexArray(0);

	// *** FBO:s for reflection and refraction
	// https://learnopengl.com/Advanced-OpenGL/Framebuffers
	// https://www.cse.chalmers.se/edu/course/TDA362/tutorials/lab5.html
	// Reflection


	

}

// Draw water
void Water::drawWater() {
	// Draw


	glBindVertexArray(VAO);

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}