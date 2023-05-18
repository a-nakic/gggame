#ifndef GRAPHICALGORITHMS_HPP
#define GRAPHICALGORITHMS_HPP


#include "common.hpp"


void ParseShader (const char *vertexShaderFilePath, const char *fragmentShaderFilePath, std::string *vertexShaderSource, std::string *fragmentShaderSource);


unsigned int CompileShader (unsigned int type, const std::string& source);


unsigned int CreateShader (const std::string& vertexShader, const std::string& fragmentShader);


void loadBackground (GLuint* background_vao, GLuint* menu_vao, SharedData* sharedData);


void drawObjectArray (GLuint texture, std::vector <GLuint>* objects);


void fillTestNodeBuffers (GLuint* vao, GLfloat x, GLfloat y);


void fillNodeCenterBuffers (GLuint* vao, GLfloat x, GLfloat y);


void fillEdgeBuffers (GLuint* vao, Node* node_1, Node* node_2);


void loadTexture (GLuint *texture, const char* path);


void fillQuadBuffers (GLuint* vao, Quad* quad, bool useTexture);


void deleteQuadBuffers (GLuint* vao);


#endif
