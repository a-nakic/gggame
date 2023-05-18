#ifndef USERINPUT_HPP
#define USERINPUT_HPP

#include "common.hpp"


double dist (Node* node_1, Node* node_2);


bool menuDetect (SharedData* sharedData);


bool nodeDetect (Node* node, SharedData* sharedData);


void newLevel (SharedData *sharedData);


void newGame (SharedData *sharedData);

#endif
