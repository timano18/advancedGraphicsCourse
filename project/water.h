#include <vector>
#include <GL/glew.h>

class Water {

public:
	Water::Water(float startX, float startY, float stopX, float stopY, float zStartPos);
	void genQuad();
	void drawWater();

private:

	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
	};
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<float> normals;

	float m_startX;
	float m_startY;
	float m_stopX;
	float m_stopY;
	float m_zStartPos;


	GLuint VAO, VBO, EBO, fbo, textureColorbuffer, rbo;


};