#ifndef COMMON_HPP
#define COMMON_HPP

#include <GL/glew.h>
#include <GL/glut.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <math.h>
#include <time.h>

#include <freetype2/ft2build.h>
#include <freetype2/freetype/freetype.h>
#include FT_FREETYPE_H


#define ACTIVE      0x01
#define INACTIVE    0x02
#define UNUSED      0x04
#define MARKED      0x08
#define MARKED_SKIP 0x10
#define BIO         0x20


struct Quad {

	std::pair <GLfloat, GLfloat> t1;
	std::pair <GLfloat, GLfloat> t2;
	std::pair <GLfloat, GLfloat> t3;
	std::pair <GLfloat, GLfloat> t4;

    GLfloat depth;
};

struct Character {
    GLuint texture;
    GLuint vao;
    glm::fvec2 Size;
    glm::fvec2 Bearing;
    double Advance;
};

struct TextObject {
	std::vector <Character>* text;
    GLuint textBackVao;
    GLuint textBackTexture;
};


struct SimpleNode {
	int value;
	std::set <SimpleNode*> neighbors;
};

struct Node {
    GLuint vao;

    uint8_t status;
    int deg;
    int value;
    double x, y;

	std::vector <Node*> parentNeighbors;
	std::vector <Node*> childNeighbors;
    Node* parentNode;
	std::vector <Node*> childNodes;
	std::set <Node*> neighbors;
	SimpleNode* simpleNode;

    TextObject* textObject;
	std::vector <Node*> similarNodes;
};

struct SharedData {

	float asp = 1080.0f / 1920.0f;
	float inv_asp = 1920.0f / 1080.0f;
	int width = 1920;
	int height = 1080;
	int window_width;
	int window_height;
	double mouse_xpos, mouse_ypos;
	double mouse_pos_multiplier_x;
	double mouse_pos_multiplier_y;
	bool clicked;
	bool clicked_leaf_node;
	int node_id = 0;
	int max_search_depth = 6;
	int max_search_width = 100000;
	int max_node_num = 15;
	bool new_game = false;
	bool new_level = false;

	int sum_user = 0, sum_pc = 0, level = 0;

	Quad* menuQuad;
	std::map <char, Character>* characters;
	Node* root;

	std::vector <GLuint>* activeNode_vaos;
	std::vector <GLuint>* inactiveNode_vaos;
	std::vector <GLuint>* nodeCenter_vaos;
	std::vector <GLuint>* edge_vaos;
	std::vector <GLuint>* nodeGlare_vaos;
	std::set <Node*>* allNodes;
	std::set <Node*>* remainingNodes;
	std::map <SimpleNode *, Node *> *nodeMap;

	std::vector <TextObject*>* node_values;
	std::vector <TextObject*>* standings;

	std::vector <Node*>* usedTreeNodes;
};


#endif
