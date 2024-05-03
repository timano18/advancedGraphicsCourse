#include <list>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

class Tree {
public:
	Tree();
	Tree(int size, std::list<char> treeList);

	void setTree(int size, std::list<char> base);
	void moveTreeBase(float x, float y, float z);
	void moveTree(float x, float y, float z);
	void generateTree(int treeType);
	void printTree();
	void Tree::toCoordsPoints();
	void Tree::toCoords2D();
	void drawTree();
	GLuint VAO, VBO, EBO;

private:
	int m_size;
	float m_thickness;
	glm::vec3 m_basePos;
	std::list<char> m_treeList;
	std::vector<glm::vec3> m_posVec;
	std::vector<unsigned int> m_posIndices;
	float m_angle;
	float m_branchLength;
	int m_branchCorrection;
};