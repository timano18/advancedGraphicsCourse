#include <GL/glew.h>
#include <vector>

class Grass {
public:
	Grass();
	Grass(unsigned int width, unsigned int height, int xPos, int yPos);
	void Grass::generateGrass();
	void Grass::DrawGrass();

private:
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
	};
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<float> normals;
	unsigned int m_width;
	unsigned int m_height;
	int m_xPos;
	int m_yPos;
	GLuint VAO, VBO, EBO;
};