#include "userInput.hpp"
#include "common.hpp"
#include "graphGeneration.hpp"
#include "statusAlgorithms.hpp"


double dist (Node* node_1, Node* node_2)
{
    double x = node_1->x - node_2->x;
    double y = node_1->y - node_2->y;

    return sqrt (x * x + y * y);
}


bool menuDetect (SharedData* sharedData)
{
    return sharedData->mouse_xpos > sharedData->menuQuad->t4.first
        && sharedData->mouse_ypos < sharedData->menuQuad->t4.second
        && sharedData->mouse_xpos < sharedData->menuQuad->t2.first
        && sharedData->mouse_ypos > sharedData->menuQuad->t2.second;
}


bool nodeDetect (Node* node, SharedData* sharedData)
{
    return (sqrt ((node->x - sharedData->mouse_xpos) * (node->x - sharedData->mouse_xpos)
    + (node->y - sharedData->mouse_ypos) * (node->y - sharedData->mouse_ypos)) < 0.05f)
    && sharedData->clicked;
}


void newLevel (SharedData *sharedData)
{
	if (sharedData->new_level) {
		sharedData->new_level = false;

		sharedData->level++;

		if (sharedData->level > 2) {
			std::string s_lost = std::string ("You Lost");
			std::string s_won = std::string ("You Won");

			if (sharedData->sum_user < sharedData->sum_pc) {
				sharedData->standings->push_back (loadTextObject (&s_lost, -0.35f, -0.01f, 4.0f, sharedData));
			} else {
				sharedData->standings->push_back (loadTextObject (&s_won, -0.35f, -0.01f, 4.0f, sharedData));
			}

		} else {
			clearGraph (sharedData);
			newGraph (sharedData);
			updateStandings (sharedData);
		}
	}
}


void newGame (SharedData *sharedData)
{
	if (sharedData->new_game) {
		sharedData->new_game = false;
		sharedData->new_level = false;

		sharedData->sum_pc = 0;
		sharedData->sum_user = 0;
		sharedData->level = 0;
		clearGraph (sharedData);
		newGraph (sharedData);
		updateStandings (sharedData);
	}
}
