#include <GL/glew.h>
#include <vector>

class Grid {
public: 
	Grid();
	Grid(unsigned int gridWidth, unsigned int gridHeight, int xStartPos, int yStartPos, float zStartPos, float cellSize, float perlinScale, float voronoiScale);

	void generateGrid();
	void DrawGrid();
	void setGridWidth(unsigned int gridWidth);
	void setGridHeight(unsigned int gridHeight);
	void voronoiPoints(int startX, int startY, int stopX, int stopY);
	GLuint getVBO();


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
	int m_xStartPos;
	int m_yStartPos;
	float m_zStartPos;
	float m_cellSize;
	float m_perlinScale;
	float m_voronoiScale;
	GLuint VAO, VBO, EBO;
};

class GridChunk {
public:
	GridChunk();
	GridChunk(int xStartPos_Chunk, int yStartPos_Chunk, int xEndPos_Chunk, int yEndPos_Chunk, float zStartPos);

	std::vector<Grid> generateChunkGrids(int xStartPosChunk, int yStartPosChunk, int xEndPosChunk, int yEndPosChunk, unsigned int gridWidth, unsigned int gridHeight, float cellSize, float zStartPos,  float perlinScale, float voronoiScale);
	void createNewStandardChunk(int startX, int startY, int endX, int endY, float zStartPos);																																								// Default
	void createNewChunk(int startX, int startY, int endX, int endY, unsigned int gridWidth, unsigned int gridHeight, float cellSize, float zStartPos, float perlinScale, float voronoiScale);																// Set parameters for the grids
	void DrawGridChunk();
	glm::vec3 gridChunkCenter();

private:
	int m_xStartPos_Chunk;
	int m_yStartPos_Chunk;
	int m_xEndPos_Chunk;
	int	m_yEndPos_Chunk;
	float m_zStartPos;
	std::vector<Grid> m_grids;
};