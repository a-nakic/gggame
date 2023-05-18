#include "treeAlgorithms.hpp"


void restoreBioDFS (Node* node)
{
    if (!(node->status & BIO)) {
        return;
    }

    node->status &= ~BIO;

    for (std::vector <Node*>::iterator it = node->childNeighbors.begin ();
        it != node->childNeighbors.end (); ++it) {

        restoreBioDFS (*it);
    }
}


void setNodeStatus (Node* node, SharedData* sharedData)
{
    int usedNeighborsNum = 0;

    if (nodeDetect (node, sharedData)) {
        if (node->status & ACTIVE) {
			sharedData->clicked_leaf_node = true;
			sharedData->sum_user += node->value;
            node->status |= UNUSED;
        }

    }

    for (std::vector <Node*>::iterator it = node->childNodes.begin ();
        it != node->childNodes.end (); ++it) {

        if (nodeDetect (*it, sharedData) && ((*it)->status & ACTIVE)) {
			sharedData->clicked_leaf_node = true;
            (*it)->status &= ~(ACTIVE | INACTIVE);
			sharedData->sum_user += (*it)->value;
            (*it)->status |= UNUSED;
        }

        if (!((*it)->status & UNUSED)) {
            usedNeighborsNum++;
        }
    }

    if (node->parentNode != NULL ){
        if (nodeDetect (node->parentNode, sharedData) && node->parentNode->status & ACTIVE) {
			sharedData->clicked_leaf_node = true;
			sharedData->sum_user += node->parentNode->value;
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
		std::string str;
        str.push_back ((char) (node->value / 10 + '0'));
        str.push_back ((char) (node->value % 10 + '0'));

        TextObject* newTextObject = loadTextObject (&str, node->x - 0.03f, node->y - 0.02f, 1.2f, sharedData);
        sharedData->node_values->push_back (newTextObject);
        if (node->textObject != NULL) {
            deleteTextObject (node->textObject);
        }
        node->textObject = newTextObject;
    }
}


void loadTreeVaosDFS (Node* node, SharedData* sharedData)
{
    if (node->status & BIO) {
        return;
    }

    setNodeStatus (node, sharedData);

    if (node->status & ACTIVE) {
        sharedData->activeNode_vaos->push_back (node->vao);
    } else if (node->status & INACTIVE) {
        sharedData->inactiveNode_vaos->push_back (node->vao);
    }

    node->status |= BIO;

    for (std::vector <Node*>::iterator it = node->childNodes.begin ();
        it != node->childNodes.end (); ++it) {

        loadTreeVaosDFS (*it, sharedData);
    }
}


void getNodesDFS (Node* node, SharedData* sharedData)
{
    if (node->status & BIO) {
        return;
    }

    node->status |= BIO;

    if (!(node->status & UNUSED)) {
        sharedData->usedTreeNodes->push_back (node);
    }

    for (std::vector <Node*>::iterator it = node->childNodes.begin ();
    it != node->childNodes.end (); ++it) {

        getNodesDFS (*it, sharedData);
    }
}
