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
#include <math.h>
#include <time.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define STB_IMAGE_IMPLEMENTATION
#include "Dependencies/std_image.h"

#define ACTIVE      0x01
#define INACTIVE    0x02
#define UNUSED      0x04
#define MARKED      0x08
#define MARKED_SKIP 0x10
#define BIO         0x20

using namespace std;


struct Node {
    GLuint vao;
    
    uint8_t status;
    int deg;
    double x, y;

    vector <Node*> parentNeighbors;
    vector <Node*> childNeighbors;
    Node* parentNode;
    vector <Node*> childNodes;

    vector <Node*> similarNodes;
};

struct Quad {
    pair <GLfloat, GLfloat> t1;
    pair <GLfloat, GLfloat> t2;
    pair <GLfloat, GLfloat> t3;
    pair <GLfloat, GLfloat> t4;

    GLfloat depth;
};

struct Character {
    unsigned int TextureID;
    glm::ivec2   Size;
    glm::ivec2   Bearing;
    unsigned int Advance;
};

float asp = 1080.0f / 1920.0f;
float inv_asp = 1920.0f / 1080.0f;
int width;
int height;
double mouse_xpos, mouse_ypos;
bool clicked;

Quad* menuQuad;

Node* root;
vector <GLuint>* activeNode_vaos;
vector <GLuint>* inactiveNode_vaos;
vector <GLuint>* nodeCenter_vaos;
vector <GLuint>* edge_vaos;
vector <GLuint>* nodeGlare_vaos;

map <char, Character> characters;


double dist (Node* node_1, Node* node_2)
{
    double x = node_1->x - node_2->x;
    double y = node_1->y - node_2->y;

    return sqrt (x * x + y * y); 
}


bool menuDetect ()
{
    return mouse_xpos > menuQuad->t4.first 
        && mouse_ypos < menuQuad->t4.second
        && mouse_xpos < menuQuad->t2.first
        && mouse_ypos > menuQuad->t2.second;
}


bool nodeDetect (Node* node)
{
    return (sqrt ((node->x - mouse_xpos) * (node->x - mouse_xpos)
    + (node->y - mouse_ypos) * (node->y - mouse_ypos)) < 0.05f)
    && clicked;
}


void restoreBioDFS (Node* node)
{
    if (!(node->status & BIO)) {
        return;
    }

    node->status &= ~BIO;

    for (vector <Node*>::iterator it = node->childNeighbors.begin ();
        it != node->childNeighbors.end (); ++it) {

        restoreBioDFS (*it);
    }
}


void setNodeStatus (Node* node)
{
    int usedNeighborsNum = 0;

    if (nodeDetect (node) && node->status & ACTIVE) {
        node->status |= UNUSED;
    }

    for (vector <Node*>::iterator it = node->childNodes.begin ();
        it != node->childNodes.end (); ++it) {

        if (nodeDetect (*it) && ((*it)->status & ACTIVE)) {
            (*it)->status &= ~(ACTIVE | INACTIVE);
            (*it)->status |= UNUSED;
        }

        if (!((*it)->status & UNUSED)) {
            usedNeighborsNum++;
        }
    }

    if (node->parentNode != NULL ){
        if (nodeDetect (node->parentNode) && node->parentNode->status & ACTIVE) {
            node->parentNode->status &= ~(ACTIVE | INACTIVE);
            node->parentNode->status |= UNUSED;
        }

        if (!(node->parentNode->status & UNUSED)) {
            usedNeighborsNum++;
        }
    }

    if (usedNeighborsNum > 1) {
        node->status &= ~ACTIVE;
        node->status |= INACTIVE;
    } else {
        node->status &= ~INACTIVE;
        node->status |= ACTIVE;
    }

    if (node->status & UNUSED) {
        node->status &= ~(ACTIVE | INACTIVE);
    }
}


void loadTreeVaosDFS (Node* node)
{
    if (node->status & BIO) {
        return;
    }

    setNodeStatus (node);

    if (node->status & ACTIVE) {
        activeNode_vaos->push_back (node->vao);
    } else if (node->status & INACTIVE) {
        inactiveNode_vaos->push_back (node->vao);
    }

    node->status |= BIO;

    for (vector <Node*>::iterator it = node->childNodes.begin ();
        it != node->childNodes.end (); ++it) {

        loadTreeVaosDFS (*it);
    }
}


static void key_callback (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose (window, GLFW_TRUE);
    }
}


static void cursor_position_callback (GLFWwindow* window, double xpos, double ypos)
{
    mouse_xpos = (xpos - width / 2) / (width / 2) * inv_asp;
    mouse_ypos = (height / 2 - ypos) / (height / 2);

    //printf ("%lf, %lf\n", mouse_xpos, mouse_ypos);
}


static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        clicked = true;
        
        activeNode_vaos->clear ();
        inactiveNode_vaos->clear ();
        
        loadTreeVaosDFS (root);
        restoreBioDFS (root);

        clicked = false;
    }
}


void framebuffer_size_callback(GLFWwindow* window, int local_width, int local_height)
{
    width = local_width;
    height = local_height;

    glViewport(0, 0, width, height);
}


static void ParseShader (const char *vertexShaderFilePath, const char *fragmentShaderFilePath, std::string *vertexShaderSource, std::string *fragmentShaderSource)
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


static unsigned int CompileShader (unsigned int type, const std::string& source)
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


static unsigned int CreateShader (const std::string& vertexShader, const std::string& fragmentShader)
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


GLFWwindow *createWindow ()
{
    if (!glfwInit ()) {
        printf ("GLFW Initialisation Failed\n");
    }

    GLFWmonitor *monitor = glfwGetPrimaryMonitor ();

    const GLFWvidmode *mode = glfwGetVideoMode (monitor);
    
    glfwWindowHint (GLFW_RED_BITS, mode -> redBits);
    glfwWindowHint (GLFW_GREEN_BITS, mode -> greenBits);
    glfwWindowHint (GLFW_BLUE_BITS, mode -> blueBits);
    glfwWindowHint (GLFW_REFRESH_RATE, mode -> refreshRate);
    
    GLFWwindow *window = glfwCreateWindow (mode -> width, mode -> height, "My Title", NULL, NULL);

    glfwGetWindowSize (window, &width, &height);
    printf ("%d %d\n", width, height);

    asp = (double) height / (double) width;
    inv_asp = (double) width / (double) height;

    glfwDestroyWindow (window);
    window = glfwCreateWindow (width, height, "My Title", NULL, NULL);

    if (!window) {
        printf ("Window Creation Failed\n");
    }

    glfwMakeContextCurrent (window);

    if (glewInit () != GLEW_OK) {
        printf ("Glew Initialisation Failed\n");
    }

    glfwSetKeyCallback (window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    printf ("%s\n", glGetString (GL_VERSION));

    return window;
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


void fillQuadBuffers (GLuint* vao, Quad* quad)
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

    GLfloat use_texture_data_0[] = {
        1.0f,
        1.0f,
        1.0f,
        1.0f
    };

    GLfloat color_buffer_data_0[] = {
        1.0f, 0.0f, 0.0f,//0
        0.0f, 1.0f, 0.0f,//1
        0.0f, 0.0f, 1.0f,//2
        0.5f, 0.2f, 0.1f //3     
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
    
    glEnableVertexAttribArray(2);
    glBindBuffer (GL_ARRAY_BUFFER, textureBuffer_0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(3);
    glBindBuffer (GL_ARRAY_BUFFER, useTextureBuffer_0);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
}


void loadBackground (GLuint* background_vao, GLuint* menu_vao)
{
    glClearColor(0.05f, 0.05f, 0.15f, 0.0f);

    Quad* backgroundQuad = new Quad;

    backgroundQuad->t1 = make_pair (-1.0f * inv_asp, -1.0f);
    backgroundQuad->t2 = make_pair (1.0f * inv_asp, -1.0f);
    backgroundQuad->t3 = make_pair (1.0f * inv_asp, 1.0f);
    backgroundQuad->t4 = make_pair (-1.0f * inv_asp, 1.0f); 

    backgroundQuad->depth = 0.0f;

    menuQuad = new Quad;

    menuQuad->t1 = make_pair (-1.0f * inv_asp + 0.01f, 1.0f - 0.05f - 0.01f);
    menuQuad->t2 = make_pair (-1.0f * inv_asp + 0.05f + 0.01f, 1.0f - 0.05f - 0.01f);
    menuQuad->t3 = make_pair (-1.0f * inv_asp + 0.05f + 0.01f, 1.0f - 0.01f);
    menuQuad->t4 = make_pair (-1.0f * inv_asp + 0.01f, 1.0f - 0.01f); 

    menuQuad->depth = -0.03f;


    fillQuadBuffers (background_vao, backgroundQuad);
    fillQuadBuffers (menu_vao, menuQuad);
}


void drawObjectArray (GLuint texture, vector <GLuint>* objects)
{
    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D, texture);

    for (vector <GLuint>::iterator it = objects->begin (); it != objects->end (); ++it) {

        glBindVertexArray (*it);
        glDrawElements (GL_TRIANGLES, 3*2, GL_UNSIGNED_INT, (void*)0);
    }
}


void fillTestNodeBuffers (GLuint* vao, GLfloat x, GLfloat y)
{
    Quad* nodeQuad = new Quad;

    nodeQuad->t1 = make_pair (-0.05f + x, -0.05f + y);
    nodeQuad->t2 = make_pair (0.05f + x, -0.05f + y);
    nodeQuad->t3 = make_pair (0.05f + x, 0.05f + y);
    nodeQuad->t4 = make_pair (-0.05f + x, 0.05f + y);

    nodeQuad->depth = -0.02f;

    fillQuadBuffers (vao, nodeQuad);
}


void fillNodeCenterBuffers (GLuint* vao, GLfloat x, GLfloat y)
{
    Quad* nodeQuad = new Quad;

    nodeQuad->t1 = make_pair (-0.05f + x, -0.05f + y);
    nodeQuad->t2 = make_pair (0.05f + x, -0.05f + y);
    nodeQuad->t3 = make_pair (0.05f + x, 0.05f + y);
    nodeQuad->t4 = make_pair (-0.05f + x, 0.05f + y);

    nodeQuad->depth = -0.015f;

    fillQuadBuffers (vao, nodeQuad);
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

    nodeQuad->t1 = make_pair (i + x2, j + y2);
    nodeQuad->t2 = make_pair (i + x1, j + y1);
    nodeQuad->t3 = make_pair (-i + x1, -j + y1);
    nodeQuad->t4 = make_pair (-i + x2, -j + y2);

    nodeQuad->depth = -0.01f;

    fillQuadBuffers (vao, nodeQuad);
}


void preCompGraph ()
{
    root->x = 0.0;
    root->y = 0.0;
    root->status = 0x00;
    root->parentNode = NULL;

    vector <Node*> nodes;
    vector <Node*> temp;
    vector <vector <Node*>> perLevelNodes;

    nodes.push_back (root);

    double fi;
    double r = 0;
    double x, y;

    for (int k = 1; k <= 13; k++) {
        fi = 2 * M_PI / (8 * k);
        r += 0.15;

        temp.clear ();

        for (int j = 0; j < 8 * k; j++) {
            x = cos (fi * j) * r;
            y = sin (fi * j) * r;

            if (y + 0.05 > 1 || y - 0.05 < -1
            || x + 0.05 > inv_asp || x - 0.05 < -inv_asp) {
                continue;
            }

            Node* newNode = new Node;

            newNode->x = x;
            newNode->y = y;
            newNode->status = 0x00;
            newNode->parentNode = NULL;

            temp.push_back (newNode);

            for (vector <Node*>::iterator it = nodes.begin ();
                it != nodes.end (); ++it) {
                
                if (dist (*it, newNode) < 0.23) {
                    (*it)->childNeighbors.push_back (newNode);
                    newNode->parentNeighbors.push_back (*it);
                }
            }            
        }

        perLevelNodes.push_back (nodes);

        nodes.clear ();
        nodes = temp;        
    }

    perLevelNodes.push_back (nodes);
}


void genTreeBuffersDFS (Node* node)
{
    if (node->status & BIO) {
        return;
    }

    fillTestNodeBuffers (&(node->vao), node->x, node->y);

    GLuint* nodeCenter_vao = new GLuint; 
    fillNodeCenterBuffers (nodeCenter_vao, node->x, node->y);

    nodeCenter_vaos->push_back (*nodeCenter_vao);

    node->status |= BIO;

    for (vector <Node*>::iterator it = node->childNodes.begin ();
        it != node->childNodes.end (); ++it) {

        GLuint* edge_vao = new GLuint;
        fillEdgeBuffers (edge_vao, node, *it);

        edge_vaos->push_back (*edge_vao);

        genTreeBuffersDFS (*it);
    }
}


double getMaxDist (Node* node, int maxDeg)
{
    set <double> distances;
    double maxDist = 0;

    for (vector <Node*>::iterator i = node->childNeighbors.begin ();
        i != node->childNeighbors.end (); ++i) {

        distances.insert (dist (node, *i));
    }

    set <double>::iterator it = distances.begin ();
    
    for (; it != distances.end (); ++it) {
        maxDeg--;
        maxDist = *it;

        if (maxDeg <= 0) {
            break;
        }
    }

    return maxDist;
}


bool markProximityNodes (Node* node, int maxDeg)
{
    set <Node*> toBeMarked;

    double maxDist = getMaxDist (node, maxDeg);

    for (vector <Node*>::iterator i = node->childNeighbors.begin ();
        i != node->childNeighbors.end (); ++i) {
    
        if ((*i)->status & MARKED) {
            return false;
        }

        if (dist (node, *i) > maxDist) {
            continue;
        }

        toBeMarked.insert (*i);
    }

    for (set <Node*>::iterator it = toBeMarked.begin ();
        it != toBeMarked.end (); ++it) {

        (*it)->status |= MARKED;
    }

    return true;
}


//TODO geometrically correct distrubution
void distributeChildNodes (Node* node, int maxDeg)
{
    double maxDist = getMaxDist (node, maxDeg);

    vector <Node*> tempChildNodes;

    for (vector <Node*>::iterator it = node->childNeighbors.begin ();
        it != node->childNeighbors.end (); ++it) {

        if (tempChildNodes.size () >= maxDeg) {
            break;
        }

        if (dist (node, *it) <= maxDist + 1e-10) {
            tempChildNodes.push_back (*it);
        }
    }

    while (!tempChildNodes.empty ()) {
        if (node->childNodes.size () >= node->deg
            || node->childNodes.size () >= node->childNeighbors.size ()) {
            break;
        }

        int randIt = rand () % (int) tempChildNodes.size ();

        node->childNodes.push_back (tempChildNodes[randIt]);
        tempChildNodes[randIt]->parentNode = node;

        tempChildNodes.erase (tempChildNodes.begin () + randIt);
    }
}


void genTestNodes ()
{
    vector <Node*> activeNodes;
    vector <Node*> chosenNodes;
    vector <Node*> temp;

    preCompGraph ();

    activeNodes.push_back (root);

    srand (time (NULL));

    while (!activeNodes.empty ()) {
        int maxDeg = 0;

        for (vector <Node*>::iterator i = activeNodes.begin ();
            i != activeNodes.end (); ++i) {

            maxDeg = max (maxDeg, (int) (*i)->childNeighbors.size ());
        }

        if (maxDeg > 0) {
            maxDeg = rand () % maxDeg + 1;
        }
            
        chosenNodes.clear ();

        while (!activeNodes.empty ()) {
            int it = rand () % activeNodes.size ();

            activeNodes[it]->status |= MARKED_SKIP;

            if (markProximityNodes (activeNodes[it], maxDeg)) {
                chosenNodes.push_back (activeNodes[it]);
            }

            temp.clear ();
            for (vector <Node*>::iterator i = activeNodes.begin ();
                i != activeNodes.end (); ++i) {

                if (!((*i)->status & (MARKED_SKIP))) {
                    temp.push_back (*i);
                }
            }
            activeNodes = temp;
        }

        for (vector <Node*>::iterator i = chosenNodes.begin ();
            i != chosenNodes.end (); ++i) {

            int tempMod = min ((int) (*i)->childNeighbors.size (), maxDeg);
            
            if (tempMod > 0) {
                (*i)->deg = rand () % tempMod + 1;
            } else {
                (*i)->deg = 0;
            }

            distributeChildNodes (*i, maxDeg);

            for (vector <Node*>::iterator j = (*i)->childNodes.begin ();
                j != (*i)->childNodes.end (); ++j) {

                activeNodes.push_back (*j);
                (*j)->parentNode = *i;
            }
        }
    }

    genTreeBuffersDFS (root);
    restoreBioDFS (root);
    loadTreeVaosDFS (root);
    restoreBioDFS (root);
}


int main ()
{
    root = new Node;
    activeNode_vaos = new vector <GLuint>;
    inactiveNode_vaos = new vector <GLuint>;
    nodeCenter_vaos = new vector <GLuint>;
    edge_vaos = new vector <GLuint>;

    GLFWwindow *window = createWindow ();
    GLuint background_vao,
        menu_vao;

    GLuint texture_backbround, 
        texture_activeNode, 
        texture_inactiveNode, 
        texture_nodeCenter, 
        texture_edge,
        texture_inactiveMenu,
        texture_activeMenu;

    loadTexture (&texture_backbround, "res/textures/background.png");
    loadTexture (&texture_activeNode, "res/textures/outer_active.png");
    loadTexture (&texture_inactiveNode, "res/textures/outer_inactive.png");
    loadTexture (&texture_nodeCenter, "res/textures/node_center.png");
    loadTexture (&texture_edge, "res/textures/edge.png");
    loadTexture (&texture_inactiveMenu, "res/textures/menu_inactive.png");
    loadTexture (&texture_activeMenu, "res/textures/menu_active.png");

    loadBackground (&background_vao, &menu_vao);
    
    genTestNodes ();

    std::string vertexShader, fragmentShader;
    ParseShader ("res/shaders/vertex.shader", "res/shaders/fragment.shader", &vertexShader, &fragmentShader);
    unsigned int shader = CreateShader (vertexShader, fragmentShader);
    glUseProgram (shader);

	GLuint matrixID = glGetUniformLocation(shader, "MVP");

    glm::mat4 model = glm::mat4 (glm::vec4 (asp, 0.0, 0.0, 0.0),
                                 glm::vec4 (0.0, 1.0, 0.0, 0.0),
                                 glm::vec4 (0.0, 0.0, 1.0, 0.0),
                                 glm::vec4 (0.0, 0.0, 0.0, 1.0));

    glm::mat4 MVP = model;

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LESS);

    glEnable (GL_ALPHA_TEST);
    glAlphaFunc (GL_GREATER, 0);

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation (GL_FUNC_ADD);

    FT_Library ft;
    if (FT_Init_FreeType (&ft)) {
        printf ("ERROR::FREETYPE: Could not init FreeType Library\n");
        return -1;
    }

    FT_Face face;
    if (FT_New_Face (ft, "/usr/share/fonts/truetype/ubuntu/Ubuntu-MI.ttf", 0, &face)) {
        printf ("ERROR::FREETYPE: Failed to load font\n");  
        return -1;
    }

    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    
    for (unsigned char c = 0; c < 128; c++) {

        if (FT_Load_Char (face, c, FT_LOAD_RENDER)) {
            printf ("ERROR::FREETYTPE: Failed to load Glyph\n");
            continue;
        }

        unsigned int texture;
        glGenTextures (1, &texture);
        glBindTexture (GL_TEXTURE_2D, texture);
        glTexImage2D (
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture, 
            glm::ivec2 (face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2 (face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        characters.insert (pair<char, Character>(c, character));
    }

    glPixelStorei (GL_PACK_ALIGNMENT, 4);


    while (!glfwWindowShouldClose (window)) {

        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);

        glActiveTexture (GL_TEXTURE0);
        glBindTexture (GL_TEXTURE_2D, texture_backbround);
        glBindVertexArray (background_vao);
        glDrawElements (GL_TRIANGLES, 3*2, GL_UNSIGNED_INT, (void*)0);

        if (menuDetect ()) {
            glBindTexture (GL_TEXTURE_2D, texture_activeMenu);    
        } else {
            glBindTexture (GL_TEXTURE_2D, texture_inactiveMenu);
        }
        glBindVertexArray (menu_vao);
        glDrawElements (GL_TRIANGLES, 3*2, GL_UNSIGNED_INT, (void*)0);

        drawObjectArray (texture_edge, edge_vaos);
        drawObjectArray (texture_nodeCenter, nodeCenter_vaos);
        drawObjectArray (texture_activeNode, activeNode_vaos);
        drawObjectArray (texture_inactiveNode, inactiveNode_vaos);

        glfwSwapBuffers (window);
        glfwPollEvents ();
    }

    glDeleteProgram (shader);

    glfwDestroyWindow (window);
    glfwTerminate ();

    return 0;
}