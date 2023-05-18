#include "common.hpp"
#include "glfwCallbacks.hpp"
#include "graphGeneration.hpp"
#include "turnComputeAlgorithms.hpp"
#include "userInput.hpp"


int main ()
{
	SharedData sharedData;
	sharedData.max_node_num = 7;

    sharedData.characters = new std::map <char, Character>;

	initGraph (&sharedData);

    GLFWwindow *window = createWindow (&sharedData);
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

    initText (&sharedData);

    loadBackground (&background_vao, &menu_vao, &sharedData);

	std::string vertexShader, fragmentShader;
    ParseShader ("res/shaders/vertex.shader", "res/shaders/fragment.shader", &vertexShader, &fragmentShader);
    unsigned int shader = CreateShader (vertexShader, fragmentShader);
    glUseProgram (shader);

	GLuint matrixID = glGetUniformLocation(shader, "MVP");

    glm::mat4 model = glm::mat4 (glm::vec4 (sharedData.asp, 0.0, 0.0, 0.0),
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

		newLevel (&sharedData);
		newGame (&sharedData);

		if (sharedData.level == 0) {
			sharedData.max_search_depth = 2;
		} else if (sharedData.level == 1) {
			sharedData.max_search_depth = 4;
		} else if (sharedData.level == 2) {
			sharedData.max_search_depth = 6;
		}

        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniformMatrix4fv (matrixID, 1, GL_FALSE, &MVP[0][0]);

        glActiveTexture (GL_TEXTURE0);
        glBindTexture (GL_TEXTURE_2D, texture_backbround);
        glBindVertexArray (background_vao);
        glDrawElements (GL_TRIANGLES, 3*2, GL_UNSIGNED_INT, (void*)0);

        if (menuDetect (&sharedData)) {
            glBindTexture (GL_TEXTURE_2D, texture_activeMenu);
        } else {
            glBindTexture (GL_TEXTURE_2D, texture_inactiveMenu);
        }
        glBindVertexArray (menu_vao);
        glDrawElements (GL_TRIANGLES, 3*2, GL_UNSIGNED_INT, (void*)0);

        drawObjectArray (texture_edge, sharedData.edge_vaos);
        drawObjectArray (texture_nodeCenter, sharedData.nodeCenter_vaos);
        drawObjectArray (texture_activeNode, sharedData.activeNode_vaos);
        drawObjectArray (texture_inactiveNode, sharedData.inactiveNode_vaos);

        for (std::vector <TextObject*>::iterator it = sharedData.node_values->begin (); it != sharedData.node_values->end (); ++it) {

            renderText (*it);
        }

        for (std::vector <TextObject*>::iterator it = sharedData.standings->begin ();
        it != sharedData.standings->end (); ++it) {

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
