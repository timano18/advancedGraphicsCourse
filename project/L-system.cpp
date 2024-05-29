#include <GL/glew.h>

#include <list>
#include <iterator>
#include "L-system.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>
#include <stack> 

#include <iostream>

#define PI 3.14159265

Tree::Tree() {
	m_treeList = {};
	m_basePos = { 0,0,0 };
}

Tree::Tree(int size, std::list<char> base) {
	m_size = size;
	m_treeList = base;
	m_basePos = { 0,0,0 };
}

// *** Set values for a tree ***
void Tree::setTree(int size, std::list<char> base) {
	m_size = size;
	m_treeList = base;
}

// *** move starting point before generating tree
void Tree::moveTreeBase(float x, float y, float z) {
	m_basePos = {x,y,z};
}

// Move entire tree by (x,y,z)
void Tree::moveTree(float x, float y, float z) {
	// TODO
}


// #######################################################################################################################################################################################

// *** Generate tree ***
void Tree::generateTree(int treeType) {

	std::list<char> updatedTree;

	// // Parameters: TreeType 1
	if (treeType == 1) {
		//m_size = 3;
		m_angle = 22.5;
		m_branchLength = 10;
		m_thickness = 1.0;
		m_treeList = { 'B' };
		m_branchCorrection = 0;
	}

	// // Parameters: TreeType 2
	if (treeType == 2) {
		//m_size = 3;
		m_angle = 22.5;
		m_branchLength = 1;
		m_thickness = 0.1;
		m_treeList = { 'A' };
		m_branchCorrection = 0;
	}

	// // Parameters: TreeType 3
	if (treeType == 3) {
		//m_size = 3;
		m_angle = 25.7;
		m_branchLength = 1;
		m_thickness = 0.1;
		m_treeList = { 'B' };
		m_branchCorrection = 0;
	}

	// // Parameters: TreeType 4
	if (treeType == 4) {
		//m_size = 2;
		m_angle = 22.5;
		m_branchLength = 1;
		m_thickness = 0.2;
		m_treeList = { 'B' };
		m_branchCorrection = 0;
	}

	// // Parameters: TreeType 5
	if (treeType == 5) {
		//m_size = 2;
		//m_angle = 22.5;
		m_angle = 25.7;
		m_branchLength = 1;
		m_thickness = 0.2;
		m_treeList = { 'A' };
		m_branchCorrection = 0;
	}

	// For size
	for (int i = 0; i < m_size; i++) {

		updatedTree = {};

		// TreeType 1
		if (treeType == 1) {

			// RULES: A -> AA & B -> A[-B][+B]

			// Each character
			std::list<char>::iterator it; // Pointer
			for (it = m_treeList.begin(); it != m_treeList.end(); it++) {

				if (*it == 'A') {
					updatedTree.push_back('A');
					updatedTree.push_back('A');
				}
				else
				if (*it == 'B') {
					updatedTree.push_back('A');

					updatedTree.push_back('[');
					updatedTree.push_back('-');
					updatedTree.push_back('B');
					updatedTree.push_back(']');

					updatedTree.push_back('[');
					updatedTree.push_back('+');
					updatedTree.push_back('B');
					updatedTree.push_back(']');
				}
				else
				if (*it == '[') {
					updatedTree.push_back('[');
				}
				else
				if (*it == ']') {
					updatedTree.push_back(']');
				}
				else
				if (*it == '-') {
					updatedTree.push_back('-');
				}
				else
				if (*it == '+') {
					updatedTree.push_back('+');
				}
			}
		}

		// TreeType 2
		if (treeType == 2) {

			// RULES: F -> FF-[-F+F+F]+[+F-F-F]

			// Each character
			std::list<char>::iterator it; // Pointer
			for (it = m_treeList.begin(); it != m_treeList.end(); it++) {

				if (*it == 'A') {
					updatedTree.push_back('A');
					updatedTree.push_back('A');

					updatedTree.push_back('-');

					updatedTree.push_back('[');
					updatedTree.push_back('-');
					updatedTree.push_back('A');
					updatedTree.push_back('+');
					updatedTree.push_back('A');
					updatedTree.push_back('+');
					updatedTree.push_back('A');
					updatedTree.push_back(']');

					updatedTree.push_back('+');

					updatedTree.push_back('[');
					updatedTree.push_back('+');
					updatedTree.push_back('A');
					updatedTree.push_back('-');
					updatedTree.push_back('A');
					updatedTree.push_back('-');
					updatedTree.push_back('A');
					updatedTree.push_back(']');
				}
				else
				if (*it == '[') {
					updatedTree.push_back('[');
				}
				else
				if (*it == ']') {
					updatedTree.push_back(']');
				}
				else
				if (*it == '-') {
					updatedTree.push_back('-');
				}
				else
				if (*it == '+') {
					updatedTree.push_back('+');
				}
			}
		}

		// TreeType 3
		if (treeType == 3) {

			// RULES: X -> F[+X][-X]FX & F -> FF

			// Each character
			std::list<char>::iterator it; // Pointer
			for (it = m_treeList.begin(); it != m_treeList.end(); it++) {

				if (*it == 'A') {
					updatedTree.push_back('A');
					updatedTree.push_back('A');
				}
				else
				if (*it == 'B') {
					updatedTree.push_back('A');

					updatedTree.push_back('[');
					updatedTree.push_back('-');
					updatedTree.push_back('B');
					updatedTree.push_back(']');

					updatedTree.push_back('[');
					updatedTree.push_back('+');
					updatedTree.push_back('B');
					updatedTree.push_back(']');

					updatedTree.push_back('A');
					updatedTree.push_back('B');
				}
				else
				if (*it == '[') {
					updatedTree.push_back('[');
				}
				else
				if (*it == ']') {
					updatedTree.push_back(']');
				}
				else
				if (*it == '-') {
					updatedTree.push_back('-');
				}
				else
				if (*it == '+') {
					updatedTree.push_back('+');
				}
			}
		}

		// TreeType 4
		if (treeType == 4) {

			// RULES: X -> F-[[X]+X]+F[+FX]-X & F -> FF

			// Each character
			std::list<char>::iterator it; // Pointer
			for (it = m_treeList.begin(); it != m_treeList.end(); it++) {

				if (*it == 'A') {
					updatedTree.push_back('A');
					updatedTree.push_back('A');
				}
				else
				if (*it == 'B') {
					updatedTree.push_back('A');

					updatedTree.push_back('-');

					updatedTree.push_back('[');
					updatedTree.push_back('[');
					updatedTree.push_back('B');
					updatedTree.push_back(']');
					updatedTree.push_back('+');
					updatedTree.push_back('B');
					updatedTree.push_back(']');
					updatedTree.push_back('+');

					updatedTree.push_back('A');
					updatedTree.push_back('[');
					updatedTree.push_back('+');
					updatedTree.push_back('A');
					updatedTree.push_back('B');
					updatedTree.push_back(']');

					updatedTree.push_back('-');
					updatedTree.push_back('B');
				}
				else
				if (*it == '[') {
					updatedTree.push_back('[');
				}
				else
				if (*it == ']') {
					updatedTree.push_back(']');
				}
				else
				if (*it == '-') {
					updatedTree.push_back('-');
				}
				else
				if (*it == '+') {
					updatedTree.push_back('+');
				}
			}
		}

		// TreeType 5
		if (treeType == 5) {

			// RULES: F -> F[+F]F[-F]F

			// Each character
			std::list<char>::iterator it; // Pointer
			for (it = m_treeList.begin(); it != m_treeList.end(); it++) {

				if (*it == 'A') {
					updatedTree.push_back('A');
					updatedTree.push_back('[');
					updatedTree.push_back('+');
					updatedTree.push_back('A');
					updatedTree.push_back(']');
					updatedTree.push_back('A');
					updatedTree.push_back('[');
					updatedTree.push_back('-');
					updatedTree.push_back('A');
					updatedTree.push_back(']');
					updatedTree.push_back('A');
				}
				else
				if (*it == '[') {
					updatedTree.push_back('[');
				}
				else
				if (*it == ']') {
					updatedTree.push_back(']');
				}
				else
				if (*it == '-') {
					updatedTree.push_back('-');
				}
				else
				if (*it == '+') {
					updatedTree.push_back('+');
				}
			}
		}

		m_treeList = updatedTree;
	}
}


// #######################################################################################################################################################################################

// *** Print out tree ***
void Tree::printTree() {
	std::cout << "Tree:" << '\t';

	std::list<char>::iterator it; // Pointer
	for (it = m_treeList.begin(); it != m_treeList.end(); it++) {
		std::cout << *it << '\t';
	}
	std::cout << "(level " << m_size << ")" << '\n' << '\n';
}


// #######################################################################################################################################################################################

// *** Create coords. to draw tree
void Tree::toCoordsPoints() {
	glm::vec3 lastPos;
	std::vector<glm::vec3> posArr;

	float dir = -90.0;

	lastPos = m_basePos;

	std::stack<glm::vec3> branchPos;
	std::stack<float> branchDir;

	// For-loop for each letter
	std::list<char>::iterator it; // Pointer
	for (it = m_treeList.begin(); it != m_treeList.end(); it++) {

		if (*it == 'A') {
			lastPos = {lastPos.x, lastPos.y + (cos(dir * PI / 180) * m_branchLength), lastPos.z + (sin(dir * PI / 180) * m_branchLength) };
			posArr.push_back(lastPos);
			//std::cout << "(" << lastPos.x << ", " << lastPos.y << ", " << lastPos.z << ')' << '\n';
		}
		else
		if (*it == 'B') {
			lastPos = { lastPos.x, lastPos.y, lastPos.z };
			posArr.push_back(lastPos);
			//std::cout << "(" << lastPos.x << ", " << lastPos.y << ", " << lastPos.z << ')' << '\n';
		}
		else
		if (*it == '[') {
			branchPos.push(lastPos);
			branchDir.push(dir);
		}
		else
		if (*it == ']') {
			lastPos = branchPos.top();
			dir = branchDir.top();
			branchPos.pop();
			branchDir.pop();
		}
		else
		if (*it == '-') {
			dir = dir - m_angle;
			//std::cout << "-: New angle: " << dir << '\n';
		}
		else
		if (*it == '+') {
			dir = dir + m_angle;
			//std::cout << "+: New angle: " << dir << '\n';
		}
	}
	m_posVec = posArr;
}


// #######################################################################################################################################################################################

// *** Create coords. to draw tree in 2d
void Tree::toCoords2D() {

	// Set up variables
	glm::vec3 lastPos = m_basePos;
	std::vector<glm::vec3> posArr = {};

	std::vector<unsigned int> indices;
	unsigned int indiceIndexTop = 1;
	unsigned int indiceIndexBot;
	unsigned int botJumpSteps = 1;

	bool newBaseRoot = false;
	unsigned int newBaseIndiceIndexBot;
	unsigned int newBaseBotJumpSteps;

	bool newNormalRoot = false;
	unsigned int newNormalIndiceIndexBot;
	unsigned int newNormalBotJumpSteps;

	unsigned int branchEnds = 0;
	unsigned int branchCounter = 0;

	unsigned int branchStarts = 0;
	unsigned int branchStartsCounter = 0;


	unsigned int botRight;
	unsigned int botLeft;
	unsigned int topRight;
	unsigned int topLeft;

	float dir = -90.0;

	std::stack<glm::vec3> branchPos;
	std::stack<float> branchDir;
	std::stack<unsigned int> indicesBotPos;

	// Base of tree
	posArr.push_back({ lastPos.x, lastPos.y + m_thickness / 2, lastPos.z + (cos(dir * PI / 180) * m_thickness / 2) });
	posArr.push_back({ lastPos.x, lastPos.y - m_thickness / 2, lastPos.z - (cos(dir * PI / 180) * m_thickness / 2) });

	// For-loop for each letter
	std::list<char>::iterator it; // Pointer
	for (it = m_treeList.begin(); it != m_treeList.end(); it++) {

		if (*it == 'A' || *it == 'B') {
			lastPos = { lastPos.x, lastPos.y + (cos(dir * PI / 180) * m_branchLength), lastPos.z + (sin(dir * PI / 180) * m_branchLength) };
			posArr.push_back({ lastPos.x, lastPos.y - (sin(dir * PI / 180) * m_thickness / 2), lastPos.z + (cos(dir * PI / 180) * m_thickness / 2) });
			posArr.push_back({ lastPos.x, lastPos.y + (sin(dir * PI / 180) * m_thickness / 2), lastPos.z - (cos(dir * PI / 180) * m_thickness / 2) });

			if (newNormalRoot) {
				botJumpSteps = newNormalBotJumpSteps;
			}

			if (newBaseRoot) {
				botJumpSteps = newBaseIndiceIndexBot;
				newBaseRoot = false;
			}

			indiceIndexBot = indiceIndexTop - botJumpSteps;

			botLeft = indiceIndexBot * 2;
			botRight = botLeft + 1;
			topLeft = indiceIndexTop * 2;
			topRight = topLeft + 1;

			std::cout << "##### DRAW BRANCH #####" << '\n';
			std::cout << "NOW IndiceIndexTop: " << indiceIndexTop << '\n';
			std::cout << "NOW IndiceIndexBot: " << indiceIndexBot << '\n';
			std::cout << "Current botJumpSteps: " << botJumpSteps << '\n';
			std::cout << "#######################" << '\n' << '\n';

			// Triangle order: 1. botLeft, botRight, topLeft; 2. botRight, topRight, topLeft
			// Tri. 1
			indices.push_back(botLeft);
			indices.push_back(botRight);
			indices.push_back(topLeft);
			// Tri. 2
			indices.push_back(botRight);
			indices.push_back(topRight);
			indices.push_back(topLeft);
			

			if (newNormalRoot) {
				botJumpSteps = 1;
				newNormalRoot = false;
			}

			indiceIndexTop++;

		} 
		else
		if (*it == '[') {

			branchPos.push(lastPos);
			branchDir.push(dir);
			indicesBotPos.push(indiceIndexBot);

			//std::cout << "JUMP BACK VALUE SAVED (indiceIndexBot): " << indiceIndexBot << '\n' << '\n';

			//std::list<char>::iterator itLastElem = it;
			//itLastElem--;
			//if (*itLastElem == '[') {
			//	branchCounter = 0;
			//}

			//branchStarts++;
			//if (branchStarts > 1) {
				//branchCounter = 0;
			//	branchStarts = 0;
			//}

			//if (branchEnds == 1) {
			//	branchEnds = 0;
			//}
			
			branchEnds = 0;

		}
		else
		if (*it == ']') {

			lastPos = branchPos.top();
			dir = branchDir.top();
			indiceIndexBot = indicesBotPos.top();

			std::cout << "JUMP BACK VALUE LOAD (indiceIndexBot): " << indiceIndexBot << '\n' << '\n';

			std::cout << "NORMALROOT TOP: " << indiceIndexTop << '\n';
			std::cout << "NORMALROOT BOT: " << indiceIndexBot << '\n';

			std::list<char>::iterator itLastElem = it;
			itLastElem--;
			if (*itLastElem == 'B') {
				//newNormalBotJumpSteps = indiceIndexTop - (indiceIndexBot + 1);
				newNormalBotJumpSteps = indiceIndexTop - (indiceIndexBot + 1 + (branchCounter * m_branchCorrection));

				branchCounter = 0;
			}
			else 
			if (*itLastElem == 'A') {
				newNormalBotJumpSteps = indiceIndexTop - (indiceIndexBot + 1);
				//newNormalBotJumpSteps = indiceIndexTop - (indiceIndexBot + 1 + (branchCounter * m_branchCorrection));

				//branchCounter = 0;
			}
			else {
				newNormalBotJumpSteps = indiceIndexTop - (indiceIndexBot + 1);
				//newNormalBotJumpSteps = indiceIndexTop - (indiceIndexBot + 1 + (branchCounter * m_branchCorrection));
				
				branchCounter = 0;
			}


			newNormalRoot = true;
			//std::cout << "NEW NORMALROOT! newNormalBotJumpSteps: " << newNormalBotJumpSteps << '\n' << '\n';

			branchPos.pop();
			branchDir.pop();
			indicesBotPos.pop();

			// Reached first bransh of tree, find following branch
			if (indicesBotPos.empty() == true) {
				std::cout << "EMPTY BOT: " << indiceIndexTop << '\n';
				std::cout << "EMPTY TOP: " << indiceIndexBot << '\n';

				newBaseIndiceIndexBot = indiceIndexTop - (indiceIndexBot + 1);

				newBaseRoot = true;
				std::cout << "IndiceIndexBot IS EMPTY! newBaseIndiceIndexBot: " << newBaseIndiceIndexBot << '\n' << '\n';
				branchEnds = 0;
				//branchCounter = 0;
			}

			//branchCounter = 0;

			// OBS! Just det här med "branchEnds" kan vara väldigt klipp-o-klistrat. Tanken är att det behövs för att flyta över koordinater när man hoppar från en gren till en annan i samma kluster av grenar. Olika träd behöver då ev. ett eget värde att hoppa med
			branchEnds++;
			//std::cout << "NOW BRANCH: " << branchEnds << '\n';
			if (branchEnds > 1) {
				if (branchEnds == 2) { 
					branchCounter++;
					//branchCounter++;
					//branchEnds = 0;
					//branchCounter = 0;
					//std::cout << "BRANCH END! Tot. amount: " << branchCounter << '\n' << '\n';
				}
				if (branchEnds == 3) { 
					branchCounter++;
					//branchCounter++;
					//branchEnds = 0;
					//branchCounter = 0;
					//std::cout << "BRANCH END! Tot. amount: " << branchCounter << '\n' << '\n';
				}
				if (branchEnds == 4) { 
					branchCounter++;
					//branchCounter++;
					//branchEnds = 0;
					//branchCounter = 0;
					//std::cout << "BRANCH END! Tot. amount: " << branchCounter << '\n' << '\n';
				}
				if (branchEnds > 4) {
					//branchCounter++;
					//branchEnds = 0;
					branchCounter = 0;
					//branchCounter = 0;
					//std::cout << "BRANCH END! Tot. amount: " << branchCounter << '\n' << '\n';
				}
			}

			//branchStarts = 0;
		}
		else
		if (*it == '-') {
			//std::cout << "-: prev. ang.: " << dir <<  '\n';
			dir = dir - m_angle;
			//std::cout << "-: new. ang.: " << dir << '\n' << '\n';
		}
		else
		if (*it == '+') {
			//std::cout << "+: prev. ang.: " << dir << '\n';
			dir = dir + m_angle;
			//std::cout << "+: new. ang.: " << dir << '\n' << '\n';
		}
	}

	m_posVec = posArr;
	m_posIndices = indices;
}


// #######################################################################################################################################################################################

// *** Draw tree ***
void Tree::drawTree() {

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, m_posVec.size() * sizeof(glm::vec3), &m_posVec[0], GL_STATIC_DRAW);
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_posIndices.size() * sizeof(unsigned int), m_posIndices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	glDrawElements(GL_TRIANGLES, m_posIndices.size(), GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_POINTS, 0, m_posVec.size());
}