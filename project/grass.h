#include <GL/glew.h>
#include <vector>

class Grass {
public:
	Grass();
	Grass(unsigned int width, unsigned int height, int xPos, int yPos, int num, int zPos);
	void Grass::generateGrassSquare();
	void Grass::generateGrassStar();
	void Grass::generateGrassTriangle();
	void Grass::DrawGrass();
	void Grass::initializeBuffers();
	void Grass::renderGrassOnTerrain(GLuint posBuffer, GLuint grassCount);

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
	int m_num;
	int m_zPos;
	
	float m_offsetXY;
	float m_offsetZ;
	float m_perlinOffset;
	float m_seed;

	GLuint VAO, VBO, EBO, instanceVBO;
};