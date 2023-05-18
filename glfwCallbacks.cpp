#include "glfwCallbacks.hpp"
#include "graphGeneration.hpp"
#include "statusAlgorithms.hpp"
#include "turnComputeAlgorithms.hpp"
#include <GLFW/glfw3.h>


static void key_callback (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	SharedData* sharedData = (SharedData*) glfwGetWindowUserPointer (window);

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose (window, GLFW_TRUE);
    }

	if (key == GLFW_KEY_N && action == GLFW_PRESS) {
		sharedData->new_game = true;
	}
}


static void cursor_position_callback (GLFWwindow* window, double xpos, double ypos)
{
	SharedData* sharedData = (SharedData*) glfwGetWindowUserPointer (window);

	sharedData->mouse_xpos = (xpos - sharedData->window_width / 2) / (sharedData->window_width / 2) * sharedData->mouse_pos_multiplier_x;
    sharedData->mouse_ypos = (sharedData->window_height / 2 - ypos) / (sharedData->window_height / 2) * sharedData->mouse_pos_multiplier_y;

    //printf ("%lf, %lf, %lf\n", mouse_xpos, mouse_ypos, inv_asp);
}


static void mouse_button_callback (GLFWwindow* window, int button, int action, int mods)
{
	
	SharedData* sharedData = (SharedData*) glfwGetWindowUserPointer (window);
	
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        sharedData->clicked = true;
		sharedData->clicked_leaf_node = false;

        sharedData->activeNode_vaos->clear ();
        sharedData->inactiveNode_vaos->clear ();
        sharedData->node_values->clear ();

        loadTreeVaosDFS (sharedData->root, sharedData);
        restoreBioDFS (sharedData->root);
        updateStandings (sharedData);
		updateRemainingNodes (sharedData);

        sharedData->clicked = false;

		if (sharedData->clicked_leaf_node) {
			computeTurn (sharedData);

			sharedData->activeNode_vaos->clear ();
			sharedData->inactiveNode_vaos->clear ();
			sharedData->node_values->clear ();

			loadTreeVaosDFS (sharedData->root, sharedData);
			restoreBioDFS (sharedData->root);
			updateStandings (sharedData);
			updateRemainingNodes (sharedData);

			printf ("jos ostalo %d\n", (int) sharedData->remainingNodes->size ());
			if (sharedData->remainingNodes->size () <= 0) {
				sharedData->new_level = true;
			}
		}
    }
}


void framebuffer_size_callback (GLFWwindow* window, int local_width, int local_height)
{
	SharedData* sharedData = (SharedData*) glfwGetWindowUserPointer (window);
    int newHeight, newWidth;
    int newx, newy;

	//std::cout << sharedData->inv_asp << std::endl;

    if (local_height * sharedData->inv_asp < local_width) {
        newHeight = local_height;
        newWidth = local_height * sharedData->inv_asp;
        newx = (local_width - newWidth) / 2;
        newy = 0;
        sharedData->mouse_pos_multiplier_x = local_width / (double) newWidth * sharedData->inv_asp;
        sharedData->mouse_pos_multiplier_y = 1.0f;
    } else {
        newWidth = local_width;
        newHeight = local_width * sharedData->asp;
        newx = 0;
        newy = (local_height - newHeight) / 2;
        sharedData->mouse_pos_multiplier_x = sharedData->inv_asp;
        sharedData->mouse_pos_multiplier_y = local_height / (double) newHeight;
    }

    sharedData->window_height = local_height;
    sharedData->window_width = local_width;

    glViewport(newx, newy, newWidth, newHeight);
}


GLFWwindow *createWindow (SharedData* sharedData)
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

    glfwGetWindowSize (window, &sharedData->window_width, &sharedData->window_height);

    glfwDestroyWindow (window);
    window = glfwCreateWindow (sharedData->width, sharedData->height, "My Title", NULL, NULL);
	glfwSetWindowUserPointer (window, sharedData);

    if (!window) {
        printf ("Window Creation Failed\n");
    }

    glfwMakeContextCurrent (window);

    if (glewInit () != GLEW_OK) {
        printf ("Glew Initialisation Failed\n");
    }

    glfwSetKeyCallback (window, key_callback);
    glfwSetCursorPosCallback (window, cursor_position_callback);
    glfwSetMouseButtonCallback (window, mouse_button_callback);
    glfwSetFramebufferSizeCallback (window, framebuffer_size_callback);

    printf ("%s\n", glGetString (GL_VERSION));

    return window;
}
