#include <GL/glew.h>

#include <list>

class Tree {
public:
	Tree();
	Tree(int size, std::list<char> treeList);

	void setTree(int size, std::list<char> base);
	void generateTree();
	void printTree();
	void drawTree();

private:
	int m_size;
	std::list<char> m_treeList;
};