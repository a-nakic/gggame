#include "graphGeneration.hpp"
#include "common.hpp"
#include "statusAlgorithms.hpp"
#include "turnComputeAlgorithms.hpp"
#include <cmath>
#include <cstdint>


void preCompGraph (SharedData* sharedData)
{
    sharedData->root->x = 0.0;
    sharedData->root->y = 0.0;
    sharedData->root->status = 0x00;
    sharedData->root->parentNode = NULL;
    sharedData->root->textObject = NULL;

	sharedData->allNodes->insert (sharedData->root);

	std::vector <Node*> nodes;
	std::vector <Node*> temp;
	std::vector <std::vector <Node*>> perLevelNodes;

    nodes.push_back (sharedData->root);

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
            || x + 0.05 > sharedData->inv_asp || x - 0.05 < -sharedData->inv_asp) {
                continue;
            }

            Node* newNode = new Node;

            newNode->x = x;
            newNode->y = y;
            newNode->status = 0x00;
            newNode->parentNode = NULL;
            newNode->textObject = NULL;

			sharedData->allNodes->insert (newNode);

            temp.push_back (newNode);

            for (std::vector <Node*>::iterator it = nodes.begin ();
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


void genTreeBuffersDFS (Node* node, SharedData* sharedData)
{
    if (node->status & BIO) {
        return;
    }

    fillTestNodeBuffers (&(node->vao), node->x, node->y);

    GLuint* nodeCenter_vao = new GLuint;
    fillNodeCenterBuffers (nodeCenter_vao, node->x, node->y);

    sharedData->nodeCenter_vaos->push_back (*nodeCenter_vao);

    node->status |= BIO;

    for (std::vector <Node*>::iterator it = node->childNodes.begin ();
        it != node->childNodes.end (); ++it) {

        GLuint* edge_vao = new GLuint;
        fillEdgeBuffers (edge_vao, node, *it);

        sharedData->edge_vaos->push_back (*edge_vao);

        genTreeBuffersDFS (*it, sharedData);
    }
}


double getMaxDist (Node* node, int maxDeg)
{
	std::set <double> distances;
    double maxDist = 0;

    for (std::vector <Node*>::iterator i = node->childNeighbors.begin ();
        i != node->childNeighbors.end (); ++i) {

        distances.insert (dist (node, *i));
    }

	std::set <double>::iterator it = distances.begin ();

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
	std::set <Node*> toBeMarked;

    double maxDist = getMaxDist (node, maxDeg);

    for (std::vector <Node*>::iterator i = node->childNeighbors.begin ();
        i != node->childNeighbors.end (); ++i) {

        if ((*i)->status & MARKED) {
            return false;
        }

        if (dist (node, *i) > maxDist) {
            continue;
        }

        toBeMarked.insert (*i);
    }

    for (std::set <Node*>::iterator it = toBeMarked.begin ();
        it != toBeMarked.end (); ++it) {

        (*it)->status |= MARKED;
    }

    return true;
}


//TODO geometrically correct distrubution
void distributeChildNodes (Node* node, int maxDeg)
{
    double maxDist = getMaxDist (node, maxDeg);

	std::vector <Node*> tempChildNodes;

    for (std::vector <Node*>::iterator it = node->childNeighbors.begin ();
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


void genTestNodes (SharedData* sharedData)
{
	std::vector <Node*> activeNodes;
	std::vector <Node*> chosenNodes;
	std::vector <Node*> temp;

    activeNodes.push_back (sharedData->root);
	sharedData->remainingNodes->insert (sharedData->root);

    srand (time (NULL));

    while (!activeNodes.empty ()) {
        int maxDeg = 0;

        for (std::vector <Node*>::iterator i = activeNodes.begin ();
            i != activeNodes.end (); ++i) {

            maxDeg = std::max (maxDeg, (int) (*i)->childNeighbors.size ());

            (*i)->value = rand () % 20;
			//(*i)->value = sharedData->node_id;
			sharedData->node_id++;
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
            for (std::vector <Node*>::iterator i = activeNodes.begin ();
                i != activeNodes.end (); ++i) {

                if (!((*i)->status & (MARKED_SKIP))) {
                    temp.push_back (*i);
                }
            }
            activeNodes = temp;
        }

        for (std::vector <Node*>::iterator i = chosenNodes.begin ();
            i != chosenNodes.end (); ++i) {

            int tempMod = std::min ((int) (*i)->childNeighbors.size (), maxDeg);

            if (tempMod > 0) {
                (*i)->deg = rand () % tempMod + 1;
            } else {
                (*i)->deg = 0;
            }

			maxDeg = std::min (maxDeg, (int) (sharedData->max_node_num - sharedData->remainingNodes->size () + sharedData->level * 5));

            distributeChildNodes (*i, maxDeg);

            for (std::vector <Node*>::iterator j = (*i)->childNodes.begin ();
                j != (*i)->childNodes.end (); ++j) {

				sharedData->remainingNodes->insert (*j);
				(*i)->neighbors.insert (*j);
				(*j)->neighbors.insert (*i);

                activeNodes.push_back (*j);
                (*j)->parentNode = *i;
            }
        }
    }

    genTreeBuffersDFS (sharedData->root, sharedData);
    restoreBioDFS (sharedData->root);
    loadTreeVaosDFS (sharedData->root, sharedData);
    restoreBioDFS (sharedData->root);
}


void initGraph (SharedData *sharedData)
{
    sharedData->root = new Node;
    sharedData->activeNode_vaos = new std::vector <GLuint>;
    sharedData->inactiveNode_vaos = new std::vector <GLuint>;
    sharedData->nodeCenter_vaos = new std::vector <GLuint>;
    sharedData->edge_vaos = new std::vector <GLuint>;
    sharedData->node_values = new std::vector <TextObject*>;
    sharedData->standings = new std::vector <TextObject*>;
	sharedData->allNodes = new std::set <Node*>;
	sharedData->remainingNodes = new std::set <Node*>;
	sharedData->nodeMap = new std::map <SimpleNode *, Node *>;
    sharedData->usedTreeNodes = new std::vector <Node*>;

    preCompGraph (sharedData);
}


void newGraph (SharedData *sharedData)
{
	genTestNodes (sharedData);
	updateStandings (sharedData);
}


void clearGraph (SharedData *sharedData)
{
	sharedData->activeNode_vaos->clear ();
	sharedData->inactiveNode_vaos->clear ();
	sharedData->nodeCenter_vaos->clear ();
	sharedData->edge_vaos->clear ();

	for (std::set <Node *>::iterator it = sharedData->allNodes->begin (); it != sharedData->allNodes->end (); ++it) {

		Node *node = (*it);

		if (node->x == 0 && node->y == 0) {
			sharedData->root = node;
		}

		node->status = 0x00;
		node->parentNode = NULL;
		node->textObject = NULL;

		node->childNodes.clear ();
		node->neighbors.clear ();
		node->simpleNode = NULL;
		node->similarNodes.clear ();
	}

	sharedData->remainingNodes->clear ();
	sharedData->nodeMap->clear ();
	sharedData->node_values->clear ();
	sharedData->standings->clear ();
	sharedData->usedTreeNodes->clear ();
}
