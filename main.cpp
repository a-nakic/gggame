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

#define STB_IMAGE_IMPLEMENTATION
#include "Dependencies/std_image.h"

#define ACTIVE      0x01
#define INACTIVE    0x02
#define UNUSED      0x04
#define MARKED      0x08
#define MARKED_SKIP 0x10
#define BIO         0x20

using namespace std;


struct Quad {
    pair <GLfloat, GLfloat> t1;
    pair <GLfloat, GLfloat> t2;
    pair <GLfloat, GLfloat> t3;
    pair <GLfloat, GLfloat> t4;

    GLfloat depth;
};

struct Character {
    GLuint texture;
    GLuint vao;
    glm::fvec2   Size;
    glm::fvec2   Bearing;
    double Advance;
};

struct TextObject {
    vector <Character>* text;
    GLuint textBackVao;
    GLuint textBackTexture;
};

struct Node {
    GLuint vao;
    
    uint8_t status;
    int deg;
    int value;
    double x, y;

    vector <Node*> parentNeighbors;
    vector <Node*> childNeighbors;
    Node* parentNode;
    vector <Node*> childNodes;

    TextObject* textObject;
    vector <Node*> similarNodes;
};


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

int sum_user = 0, sum_pc = 0;

Quad* menuQuad;
map <char, Character>* characters;
Node* root;

vector <GLuint>* activeNode_vaos;
vector <GLuint>* inactiveNode_vaos;
vector <GLuint>* nodeCenter_vaos;
vector <GLuint>* edge_vaos;
vector <GLuint>* nodeGlare_vaos;

vector <TextObject*>* node_values;
vector <TextObject*>* standings;

vector <Node*>* usedTreeNodes;


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
    
    glEnableVertexAttribArray(2);
    glBindBuffer (GL_ARRAY_BUFFER, textureBuffer_0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(3);
    glBindBuffer (GL_ARRAY_BUFFER, useTextureBuffer_0);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
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


void deleteTextObject (TextObject* textObject)
{
    if (textObject != nullptr) {
        if (textObject->text != nullptr) {
            for (vector <Character>::iterator it = textObject->text->begin ();
            it != textObject->text->end (); ++it) {

                deleteQuadBuffers (&((*it).vao));
            }

            delete textObject->text;
        }
    
        delete textObject;
    }
}


TextObject* loadTextObject (string* text, float x, float y, float scale)
{
    TextObject* ret = new TextObject;
    vector <Character>* vec = new vector <Character>;

    float max_x = -20.0f, max_y = -20.0f, min_x = 20.0f, min_y = 20.0f;

    for (string::iterator it = text->begin ();
    it != text->end (); ++it) {

        Character ch = (*characters)[*it];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        max_x = max (max_x, xpos + w);
        max_y = max (max_y, ypos + h);
        min_x = min (min_x, xpos);
        min_y = min (min_y, ypos);

        Quad* textBox = new Quad;

        textBox->t4 = make_pair (xpos, ypos);
        textBox->t1 = make_pair (xpos, ypos + h);
        textBox->t2 = make_pair (xpos + w, ypos + h);
        textBox->t3 = make_pair (xpos + w, ypos);
        textBox->depth = -0.04f;

        GLuint* textVao = new GLuint;
        fillQuadBuffers (textVao, textBox, false);
        ch.vao = *textVao;

        vec->push_back (ch);

        delete textBox;
        delete textVao;

        x += ch.Advance * scale;
    }

    Quad* textBack = new Quad;

    textBack->t4 = make_pair (min_x - 0.01f, min_y - 0.01f);
    textBack->t1 = make_pair (min_x - 0.01f, max_y + 0.01f);
    textBack->t2 = make_pair (max_x + 0.01f, max_y + 0.01f);
    textBack->t3 = make_pair (max_x + 0.01f, min_y - 0.01f);
    textBack->depth = -0.039f;

    GLuint* backVao = new GLuint;
    fillQuadBuffers (backVao, textBack, true);

    ret->textBackVao = *backVao;
    ret->text = vec;

    return ret;
}


void initText ()
{
    FT_Library ft;
    if (FT_Init_FreeType (&ft)) {
        printf ("ERROR::FREETYPE: Could not init FreeType Library\n");
        return;
    }

    FT_Face face;
    if (FT_New_Face (ft, "res/fonts/OpenSans-Regular.ttf", 0, &face)) {
        printf ("ERROR::FREETYPE: Failed to load font\n");  
        return;
    }
    FT_Set_Pixel_Sizes(face, 0, 48); 

    for (unsigned char c = 0; c < 128; c++) {

        if (FT_Load_Char (face, c, FT_LOAD_RENDER)) {
            printf ("ERROR::FREETYTPE: Failed to load Glyph\n");
            continue;
        }
        
        glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
        
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
    
        glPixelStorei (GL_PACK_ALIGNMENT, 4);

        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            NULL, 
            glm::fvec2 (face->glyph->bitmap.width / (double) height, face->glyph->bitmap.rows / (double) height),
            glm::fvec2 (face->glyph->bitmap_left / (double) height, face->glyph->bitmap_top / (double) height),
            (face->glyph->advance.x / (double) height) / 64.0f
        };
        characters->insert (pair<char, Character>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}


void renderText (TextObject* textObject)
{
    for (vector <Character>::iterator it = textObject->text->begin ();
    it != textObject->text->end (); ++it) {

        glBindTexture (GL_TEXTURE_2D, it->texture);
        glBindVertexArray (it->vao);
        glDrawElements (GL_TRIANGLES, 3*2, GL_UNSIGNED_INT, (void*)0);  
    }
}


void updateStandings ()
{
    int pot = 10000;
    string s1 ("user"), s2 ("pc");
    string s3, s4;
    
    for (int i = 0; i < 4; i++) {
        char c1 = (char) (sum_user % pot / (pot / 10) + '0');
        char c2 = (char) (sum_pc % pot / (pot / 10) + '0');
        pot /= 10;

        s3.push_back (c1);
        s4.push_back (c2);
    }

    for (vector <TextObject*>::iterator it = standings->begin ();
    it != standings->end (); ++it) {

        deleteTextObject (*it);
    }
    standings->clear ();

    standings->push_back (loadTextObject (&s1, -inv_asp, 0.1f, 1.0f));
    standings->push_back (loadTextObject (&s3, -inv_asp, 0.05f, 1.0f));
    standings->push_back (loadTextObject (&s2, -inv_asp, -0.05f, 1.0f));
    standings->push_back (loadTextObject (&s4, -inv_asp, -0.1f, 1.0f));
}


void setNodeStatus (Node* node)
{
    int usedNeighborsNum = 0;

    if (nodeDetect (node)) {
        if (node->status & ACTIVE) {
            node->status |= UNUSED;
        }

        sum_user += node->value;
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
    } else {
        string str; 
        str.push_back((char) (node->value / 10 + '0'));
        str.push_back((char) (node->value % 10 + '0'));

        TextObject* newTextObject = loadTextObject (&str, node->x - 0.03f, node->y - 0.02f, 1.2f);
        
        node_values->push_back (newTextObject);
        if (node->textObject != NULL) {
            deleteTextObject (node->textObject);
        }
        node->textObject = newTextObject;
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


void getNodesDFS (Node* node)
{
    if (node->status & BIO) {
        return;
    }

    node->status |= BIO;

    if (!(node->status & UNUSED)) {
        usedTreeNodes->push_back (node);
    }

    for (vector <Node*>::iterator it = node->childNodes.begin ();
    it != node->childNodes.end (); ++it) {

        getNodesDFS (*it);
    }
}


int getMaxWin (int it, int skip, int userTurnsLeft, int pcTurnsLeft, int userSum, int pcSum)
{
    int retMaxWin = 0;

    if (userTurnsLeft == 0 && pcTurnsLeft == 0) {
        if (pcSum > userSum) {
            return 1;
        } else {
            return 0;
        }
    }

    if (it == skip) {
        it++;
        pcTurnsLeft--;
    }

    if (it < (int) usedTreeNodes->size ()) {

        if (pcTurnsLeft > 0) {
            pcSum += (*usedTreeNodes)[it]->value;

            retMaxWin += getMaxWin (it + 1, skip, userTurnsLeft, pcTurnsLeft - 1, userSum, pcSum);

            pcSum -= (*usedTreeNodes)[it]->value;
        }

        if (userTurnsLeft > 0) {
            userSum += (*usedTreeNodes)[it]->value;

            retMaxWin += getMaxWin (it + 1, skip, userTurnsLeft - 1, pcTurnsLeft, userSum, pcSum);

            userSum -= (*usedTreeNodes)[it]->value;
        }
    }

    return retMaxWin;
}


void computerTurn ()
{
    usedTreeNodes->clear ();

    getNodesDFS (root);
    restoreBioDFS (root);

    int maxWin = 0, temp = 0, userTurns, pcTurns, maxValue = 0;
    Node* node = NULL;

    userTurns = (int) usedTreeNodes->size () / 2;
    pcTurns = userTurns;

    if (userTurns + pcTurns < (int) usedTreeNodes->size ()) {
        pcTurns++;
    }

    for (vector <Node*>::iterator it = usedTreeNodes->begin ();
    it != usedTreeNodes->end (); ++it) {

        if (!((*it)->status & ACTIVE)) {
            continue;
        }

        if (maxValue < (*it)->value) {
            maxValue = (*it)->value;
            node = *it;
        }
    }    

    for (vector <Node*>::iterator it = usedTreeNodes->begin ();
    it != usedTreeNodes->end (); ++it) {

        if (!((*it)->status & ACTIVE)) {
            continue;
        }

        temp = getMaxWin (0, (int) (it - usedTreeNodes->begin ()), userTurns, pcTurns, sum_user, sum_pc + (*it)->value);

        printf ("%d\n", temp);

        if (maxWin < temp) {
            maxWin = temp;
            node = *it;
        }
    }

    printf ("done.\n");

    if (node != NULL) {
        node->status |= UNUSED;
        sum_pc += node->value;
    }

    loadTreeVaosDFS (root);
    restoreBioDFS (root);
}


static void key_callback (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose (window, GLFW_TRUE);
    }
}


static void cursor_position_callback (GLFWwindow* window, double xpos, double ypos)
{
    mouse_xpos = (xpos - window_width / 2) / (window_width / 2) * mouse_pos_multiplier_x;
    mouse_ypos = (window_height / 2 - ypos) / (window_height / 2) * mouse_pos_multiplier_y;

    //printf ("%lf, %lf, %lf\n", mouse_xpos, mouse_ypos, inv_asp);
}


static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        clicked = true;
        
        activeNode_vaos->clear ();
        inactiveNode_vaos->clear ();
        node_values->clear ();
        
        loadTreeVaosDFS (root);
        restoreBioDFS (root);
        updateStandings ();

        clicked = false;

        computerTurn ();

        activeNode_vaos->clear ();
        inactiveNode_vaos->clear ();
        node_values->clear ();
        
        loadTreeVaosDFS (root);
        restoreBioDFS (root);
        updateStandings ();
    }
}


void framebuffer_size_callback(GLFWwindow* window, int local_width, int local_height)
{
    int newHeight, newWidth;
    int newx, newy;

    cout << inv_asp << endl;

    if (local_height * inv_asp < local_width) {
        newHeight = local_height;
        newWidth = local_height * inv_asp;
        newx = (local_width - newWidth) / 2;
        newy = 0;
        mouse_pos_multiplier_x = local_width / (double) newWidth * inv_asp;
        mouse_pos_multiplier_y = 1.0f;
    } else {
        newWidth = local_width;
        newHeight = local_width * asp;
        newx = 0;
        newy = (local_height - newHeight) / 2;
        mouse_pos_multiplier_x = inv_asp;
        mouse_pos_multiplier_y = local_height / (double) newHeight;
    }

    window_height = local_height;
    window_width = local_width;

    glViewport(newx, newy, newWidth, newHeight);
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

    glfwGetWindowSize (window, &window_width, &window_height);

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


    fillQuadBuffers (background_vao, backgroundQuad, true);
    fillQuadBuffers (menu_vao, menuQuad, true);
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

    fillQuadBuffers (vao, nodeQuad, true);
}


void fillNodeCenterBuffers (GLuint* vao, GLfloat x, GLfloat y)
{
    Quad* nodeQuad = new Quad;

    nodeQuad->t1 = make_pair (-0.05f + x, -0.05f + y);
    nodeQuad->t2 = make_pair (0.05f + x, -0.05f + y);
    nodeQuad->t3 = make_pair (0.05f + x, 0.05f + y);
    nodeQuad->t4 = make_pair (-0.05f + x, 0.05f + y);

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

    nodeQuad->t1 = make_pair (i + x2, j + y2);
    nodeQuad->t2 = make_pair (i + x1, j + y1);
    nodeQuad->t3 = make_pair (-i + x1, -j + y1);
    nodeQuad->t4 = make_pair (-i + x2, -j + y2);

    nodeQuad->depth = -0.01f;

    fillQuadBuffers (vao, nodeQuad, true);
}


void preCompGraph ()
{
    root->x = 0.0;
    root->y = 0.0;
    root->status = 0x00;
    root->parentNode = NULL;
    root->textObject = NULL;

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
            newNode->textObject = NULL;

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

            (*i)->value = rand () % 20;
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
    characters = new map <char, Character>;

    activeNode_vaos = new vector <GLuint>;
    inactiveNode_vaos = new vector <GLuint>;
    nodeCenter_vaos = new vector <GLuint>;
    edge_vaos = new vector <GLuint>;
    node_values = new vector <TextObject*>;
    standings = new vector <TextObject*>;

    usedTreeNodes = new vector <Node*>;

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

    initText ();

    loadBackground (&background_vao, &menu_vao);

    genTestNodes ();

    updateStandings ();

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

        for (vector <TextObject*>::iterator it = node_values->begin ();
        it != node_values->end (); ++it) {
            
            renderText (*it);
        }

        for (vector <TextObject*>::iterator it = standings->begin ();
        it != standings->end (); ++it) {
            
            renderText (*it);
        }

        glfwSwapBuffers (window);
        glfwPollEvents ();
    }

    glDeleteProgram (shader);

    glfwDestroyWindow (window);
    glfwTerminate ();

    return 0;
}