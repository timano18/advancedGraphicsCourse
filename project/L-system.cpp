#include <GL/glew.h>

#include <list>
#include <iterator>
#include "L-system.h"

#include <iostream>

// *** Parameters for standard tree ***
int s_size = 5;
std::list<char> s_base = { 'A' };

// *** Rules ***																		// Skapa rule sets för oilka former
// 1. A -> AB
// 2. B -> A

Tree::Tree() {
	m_size = s_size;
	m_treeList = s_base;
}

Tree::Tree(int size, std::list<char> base) {
	m_size = size;
	m_treeList = base;
}

// *** Set values for a tree ***
void Tree::setTree(int size, std::list<char> base) {
	m_size = size;
	m_treeList = base;
}

// *** Generate tree ***																// SKAPA METODER FÖR OLIKA SÄTT ATT GENERERA (nu bara det som står i "Rules")
void Tree::generateTree() {

	std::list<char> updatedTree;

	// For size
	for (int i = 0; i < m_size; i++) {

		updatedTree = {};

		// Each character
		std::list<char>::iterator it; // Pointer
		for (it = m_treeList.begin(); it != m_treeList.end(); it++) {

			if (*it == 'A') {
				//std::cout << "A->AB from: " << *it << '\n';							// Felsökning
				updatedTree.push_back('A');
				updatedTree.push_back('B');
			}
			else
				if (*it == 'B') {
					//std::cout << "B->A from: " << *it << '\n';						// Felsökning
					updatedTree.push_back('A');
				}

		}

																						// Felsökning 
		/*
		std::list<char>::iterator it2; // Pointer
		for (it2 = updatedTree.begin(); it2 != updatedTree.end(); it2++) {
			std::cout << *it2 << '\t';
		}
		std::cout << '\n';
		*/


		m_treeList = updatedTree;
	}
}

// *** Print out tree ***
void Tree::printTree() {
	std::cout << "Tree:" << '\t';

	std::list<char>::iterator it; // Pointer
	for (it = m_treeList.begin(); it != m_treeList.end(); it++) {
		std::cout << *it << '\t';
	}
	std::cout << "(level " << m_size << ")" << '\n';
}

// *** Draw tree ***
void Tree::drawTree() {


	// ATT GÖRA!!!


}