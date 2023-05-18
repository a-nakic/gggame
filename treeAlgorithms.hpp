#ifndef TREEALGORITHMS_HPP
#define TREEALGORITHMS_HPP

#include "userInput.hpp"
#include "textProcessing.hpp"


void restoreBioDFS (Node* node);


void setNodeStatus (Node* node, SharedData* sharedData);


void loadTreeVaosDFS (Node* node, SharedData* sharedData);


void getNodesDFS (Node* node, SharedData* sharedData);


#endif
