#include "graphicAlgorithms.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "dependencies/std_image.h"


void ParseShader (const char *vertexShaderFilePath, const char *fragmentShaderFilePath, std::string *vertexShaderSource, std::string *fragmentShaderSource)
{
    FILE *shaderFile;
    char c;

    shaderFile = fopen (vertexShaderFilePath, "r");

    c = fgetc (shaderFile);
    while (c != EOF) {
        (*vertexShaderSource).push_back (c);
        c = fgetc (shaderFile);
    }

    shaderFile = fopen (fragmentShaderFilePath, "r");

    c = fgetc (shaderFile);
    while (c != EOF) {
        (*fragmentShaderSource).push_back (c);
        c = fgetc (shaderFile);
    }

    fclose (shaderFile);
}


unsigned int CompileShader (unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader (type);
    const char* src = source.c_str ();
    glShaderSource (id, 1, &src, nullptr);
    glCompileShader (id);

    int result;
    glGetShaderiv (id, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE) {
        int length;
        glGetShaderiv (id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca (length * sizeof (char));
        glGetShaderInfoLog (id, length, &length, message);

        std::cout << "Failed to compile shader" << std::endl;
        std::cout << message << std::endl;

        glDeleteShader (id);
        return 0;
    }

    return id;
}


unsigned int CreateShader (const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram ();
    unsigned int vs = CompileShader (GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader (GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader (program, vs);
    glAttachShader (program, fs);
    //glBindFragDataLocation(program, 0, "FragColor");
    glLinkProgram (program);
    glValidateProgram (program);

    glDeleteShader (vs);
    glDeleteShader (fs);

    return program;
}


void loadBackground (GLuint* background_vao, GLuint* menu_vao, SharedData* sharedData)
{
    glClearColor(0.05f, 0.05f, 0.15f, 0.0f);

    Quad* backgroundQuad = new Quad;

    backgroundQuad->t1 = std::make_pair (-1.0f * sharedData->inv_asp, -1.0f);
    backgroundQuad->t2 = std::make_pair (1.0f * sharedData->inv_asp, -1.0f);
    backgroundQuad->t3 = std::make_pair (1.0f * sharedData->inv_asp, 1.0f);
    backgroundQuad->t4 = std::make_pair (-1.0f * sharedData->inv_asp, 1.0f);

    backgroundQuad->depth = 0.0f;

    sharedData->menuQuad = new Quad;

    sharedData->menuQuad->t1 = std::make_pair (-1.0f * sharedData->inv_asp + 0.01f, 1.0f - 0.05f - 0.01f);
    sharedData->menuQuad->t2 = std::make_pair (-1.0f * sharedData->inv_asp + 0.05f + 0.01f, 1.0f - 0.05f - 0.01f);
    sharedData->menuQuad->t3 = std::make_pair (-1.0f * sharedData->inv_asp + 0.05f + 0.01f, 1.0f - 0.01f);
    sharedData->menuQuad->t4 = std::make_pair (-1.0f * sharedData->inv_asp + 0.01f, 1.0f - 0.01f);

    sharedData->menuQuad->depth = -0.03f;


    fillQuadBuffers (background_vao, backgroundQuad, true);
    fillQuadBuffers (menu_vao, sharedData->menuQuad, true);
}


void loadTexture (GLuint *texture, const char* path)
{
    int imgWidth, imgHeight, nrChannels;
    unsigned char *data = nullptr;

    data = stbi_load (path, &imgWidth, &imgHeight, &nrChannels, 0);

    glGenTextures (1, texture);
    glBindTexture (GL_TEXTURE_2D, *texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, imgWidth, imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
}


void drawObjectArray (GLuint texture, std::vector <GLuint>* objects)
{
    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D, texture);

    for (std::vector <GLuint>::iterator it = objects->begin (); it != objects->end (); ++it) {

        glBindVertexArray (*it);
        glDrawElements (GL_TRIANGLES, 3*2, GL_UNSIGNED_INT, (void*)0);
    }
}


void fillTestNodeBuffers (GLuint* vao, GLfloat x, GLfloat y)
{
    Quad* nodeQuad = new Quad;

    nodeQuad->t1 = std::make_pair (-0.05f + x, -0.05f + y);
    nodeQuad->t2 = std::make_pair (0.05f + x, -0.05f + y);
    nodeQuad->t3 = std::make_pair (0.05f + x, 0.05f + y);
    nodeQuad->t4 = std::make_pair (-0.05f + x, 0.05f + y);

    nodeQuad->depth = -0.02f;

    fillQuadBuffers (vao, nodeQuad, true);
}


void fillNodeCenterBuffers (GLuint* vao, GLfloat x, GLfloat y)
{
    Quad* nodeQuad = new Quad;

    nodeQuad->t1 = std::make_pair (-0.05f + x, -0.05f + y);
    nodeQuad->t2 = std::make_pair (0.05f + x, -0.05f + y);
    nodeQuad->t3 = std::make_pair (0.05f + x, 0.05f + y);
    nodeQuad->t4 = std::make_pair (-0.05f + x, 0.05f + y);

    nodeQuad->depth = -0.015f;

    fillQuadBuffers (vao, nodeQuad, true);
}


void fillEdgeBuffers (GLuint* vao, Node* node_1, Node* node_2)
{
    GLfloat x1, y1, x2, y2;
    GLfloat r, i, j, k;

    r = 0.05;

    x1 = node_1->x;
    y1 = node_1->y;
    x2 = node_2->x;
    y2 = node_2->y;

    k = (x1 - x2) / (y2 - y1);

    i = 1 / sqrt (1 + k * k) * r;
    j = k / sqrt (1 + k * k) * r;

    if (k > 9000.0) {
        i = 0;
        j = r;
    } else if (k < -9000.0) {
        i = 0;
        j = -r;
    }

    Quad* nodeQuad = new Quad;

    nodeQuad->t1 = std::make_pair (i + x2, j + y2);
    nodeQuad->t2 = std::make_pair (i + x1, j + y1);
    nodeQuad->t3 = std::make_pair (-i + x1, -j + y1);
    nodeQuad->t4 = std::make_pair (-i + x2, -j + y2);

    nodeQuad->depth = -0.01f;

    fillQuadBuffers (vao, nodeQuad, true);
}


void fillQuadBuffers (GLuint* vao, Quad* quad, bool useTexture)
{
    glGenVertexArrays (1, vao);

    GLfloat position_buffer_data_0[] = {
        quad->t1.first, quad->t1.second, quad->depth,
        quad->t2.first, quad->t2.second, quad->depth,
        quad->t3.first, quad->t3.second, quad->depth,
        quad->t4.first, quad->t4.second, quad->depth
    };

    GLfloat texture_buffer_data_0[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    float f = 1.0f;
    if (!useTexture) {
        f = 0.0f;
    }

    GLfloat use_texture_data_0[] = {
        f, f, f, f
    };

    GLfloat color_buffer_data_0[] = {
        0.7f, 0.7f, 0.7f,
        0.7f, 0.7f, 0.7f,
        0.7f, 0.7f, 0.7f,
        0.7f, 0.7f, 0.7f
    };

    GLuint indices_0[] = {
        0, 1, 2,
        0, 3, 2,
    };

    glBindVertexArray(*vao);

    GLuint textureBuffer_0;
    glGenBuffers (1, &textureBuffer_0);
    glBindBuffer (GL_ARRAY_BUFFER, textureBuffer_0);
    glBufferData (GL_ARRAY_BUFFER, sizeof (texture_buffer_data_0), texture_buffer_data_0, GL_STATIC_DRAW);

    GLuint colorBuffer_0;
    glGenBuffers (1, &colorBuffer_0);
    glBindBuffer (GL_ARRAY_BUFFER, colorBuffer_0);
    glBufferData (GL_ARRAY_BUFFER, sizeof (color_buffer_data_0), color_buffer_data_0, GL_STATIC_DRAW);

    GLuint positionBuffer_0;
    glGenBuffers (1, &positionBuffer_0);
    glBindBuffer (GL_ARRAY_BUFFER, positionBuffer_0);
    glBufferData (GL_ARRAY_BUFFER, sizeof (position_buffer_data_0), position_buffer_data_0, GL_STATIC_DRAW);

    GLuint useTextureBuffer_0;
    glGenBuffers (1, &useTextureBuffer_0);
    glBindBuffer (GL_ARRAY_BUFFER, useTextureBuffer_0);
    glBufferData (GL_ARRAY_BUFFER, sizeof (use_texture_data_0), use_texture_data_0, GL_STATIC_DRAW);

    GLuint ibo_0;
    glGenBuffers (1, &ibo_0);
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, ibo_0);
    glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (indices_0), indices_0, GL_STATIC_DRAW);

    glEnableVertexAttribArray (0);
    glBindBuffer (GL_ARRAY_BUFFER, positionBuffer_0);
    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray (1);
    glBindBuffer (GL_ARRAY_BUFFER, colorBuffer_0);
    glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray (2);
    glBindBuffer (GL_ARRAY_BUFFER, textureBuffer_0);
    glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray (3);
    glBindBuffer (GL_ARRAY_BUFFER, useTextureBuffer_0);
    glVertexAttribPointer (3, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
}


void deleteQuadBuffers (GLuint* vao)
{
    GLint nAttr = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nAttr);
    glBindVertexArray(*vao);

    for (int iAttr = 0; iAttr < nAttr; ++iAttr) {
        GLint vboId = 0;
        GLuint vbo;
        glGetVertexAttribiv(iAttr, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vboId);

        if (vboId > 0) {
            vbo = vboId;
            glDeleteBuffers (1, &vbo);
        }
    }

    glDeleteVertexArrays (1, vao);
}
