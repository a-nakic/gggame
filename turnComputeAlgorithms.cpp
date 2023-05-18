#include "turnComputeAlgorithms.hpp"
#include "common.hpp"


int updateRemainingNodes (SharedData *sharedData)
{
	std::set <Node *> nodes_to_remove;

	for (std::set <Node *>::iterator it = sharedData->remainingNodes->begin (); it != sharedData->remainingNodes->end (); ++it) {

		if ((*it)->status & UNUSED && (*it)->neighbors.size () <= 1) {
			nodes_to_remove.insert (*it);
		}
	}

	for (std::set <Node *>::iterator i = nodes_to_remove.begin (); i != nodes_to_remove.end (); ++i) {

		std::set <Node *>::iterator node_it;
		node_it = sharedData->remainingNodes->find (*i);

		if (node_it != sharedData->remainingNodes->end ()) {
			sharedData->remainingNodes->erase (node_it);
		}

		for (std::set <Node *>::iterator j = sharedData->remainingNodes->begin (); j != sharedData->remainingNodes->end (); ++j) {

			node_it = (*j)->neighbors.find (*i);

			if (node_it != (*j)->neighbors.end ()) {
				(*j)->neighbors.erase (node_it);
			}
		}
	}

	return 0;
}


void numOfWins (std::set <SimpleNode*> leafNodes, int currDepth, int* currWidth, int* wins, bool compTurn, int compSum, int userSum, SharedData* sharedData)
{
	if (leafNodes.size () < 1 || *currWidth >= sharedData->max_search_width) {

	} else if (currDepth >= sharedData->max_search_depth ) {

		if (compSum > userSum) {
			(*wins)++;
		}

		(*currWidth)++;

	} else if (leafNodes.size () == 1) {
		
		if (compTurn) {
			compSum += (*leafNodes.begin ())->value;
		} else {
			userSum += (*leafNodes.begin ())->value;
		}

		if (compSum > userSum) {
			(*wins)++;
		}

		(*currWidth)++;

	} else {

		std::set <SimpleNode*> tempLeafNodes = leafNodes;

		//for (std::set <SimpleNode*>::iterator it = leafNodes.begin (); it != leafNodes.end (); ++it) {

		while (!leafNodes.empty ()) {

			std::set <SimpleNode *>::iterator it = leafNodes.begin ();
			SimpleNode *neighbor_of_current_leaf;
			SimpleNode *current_leaf;
			int max_value = -1;
			int min_value = 20;

			if (!compTurn) {

				for (it = leafNodes.begin (); it != leafNodes.end (); ++it) {

					if ((*it)->value > max_value) {
						max_value = (*it)->value;
						neighbor_of_current_leaf = *((*it)->neighbors.begin ());
						current_leaf = *it;
					}
				}
			} else {

				for (it = leafNodes.begin (); it != leafNodes.end (); ++it) {

					if ((*it)->value < min_value) {
						min_value = (*it)->value;
						neighbor_of_current_leaf = *((*it)->neighbors.begin ());
						current_leaf = *it;
					}
				}
			}
			leafNodes.erase (current_leaf);

			//SimpleNode* neighbor_of_current_leaf = *((*it)->neighbors.begin ());
			//SimpleNode* current_leaf = *it;

			tempLeafNodes.erase (current_leaf);
			current_leaf->neighbors.erase (neighbor_of_current_leaf);
			neighbor_of_current_leaf->neighbors.erase (current_leaf);

			if (neighbor_of_current_leaf->neighbors.size () == 1) {

				tempLeafNodes.insert (neighbor_of_current_leaf);
			}

			if (compTurn) {
				compSum += current_leaf->value;
			} else {
				userSum += current_leaf->value;
			}

			numOfWins (tempLeafNodes, currDepth + 1, currWidth, wins, !compTurn, compSum, userSum, sharedData);

			if (compTurn) {
				compSum -= current_leaf->value;
			} else {
				userSum -= current_leaf->value;
			}

			if (neighbor_of_current_leaf->neighbors.size () == 1) {

				tempLeafNodes.erase (neighbor_of_current_leaf);
			}

			neighbor_of_current_leaf->neighbors.insert (current_leaf);
			current_leaf->neighbors.insert (neighbor_of_current_leaf);
			tempLeafNodes.insert (current_leaf);
		}
	}
}


void computeTurn (SharedData* sharedData)
{
	std::set <SimpleNode*> allSimpleNodes;
	sharedData->nodeMap->clear ();

	for (std::set <Node *>::iterator it = sharedData->remainingNodes->begin (); it != sharedData->remainingNodes->end (); ++it) {

		SimpleNode *simpleNode = new SimpleNode;
		simpleNode->value = (*it)->value;

		(*it)->simpleNode = simpleNode;
		allSimpleNodes.insert (simpleNode);

		sharedData->nodeMap->insert (std::pair <SimpleNode *, Node *> (simpleNode, *it));
	}

	for (std::set <Node *>::iterator it = sharedData->remainingNodes->begin (); it != sharedData->remainingNodes->end (); ++it) {

		for (std::set <Node *>::iterator n_it = (*it)->neighbors.begin (); n_it != (*it)->neighbors.end (); ++n_it) {

			(*it)->simpleNode->neighbors.insert ((*n_it)->simpleNode);
		}
	}

	std::set <SimpleNode *> leafNodes;

	for (std::set <SimpleNode *>::iterator it = allSimpleNodes.begin (); it != allSimpleNodes.end (); ++it) {

		if ((*it)->neighbors.size () == 1) {

			leafNodes.insert (*it);
		}
	}

	std::set <SimpleNode *> tempLeafNodes = leafNodes;
	double win_probability = -1;
	int max_sum = 0;
	Node *win_node = NULL;

	for (std::set <SimpleNode *>::iterator it = leafNodes.begin (); it != leafNodes.end (); ++it) {

		SimpleNode* current_leaf = *it;
		SimpleNode* neighbor_of_current_leaf = *((*it)->neighbors.begin ());

		tempLeafNodes.erase (current_leaf);
		neighbor_of_current_leaf->neighbors.erase (current_leaf);

		if (neighbor_of_current_leaf->neighbors.size () == 1) {

			tempLeafNodes.insert (neighbor_of_current_leaf);
		}

		int currWidth = 0;
		int wins = 0;
		numOfWins (tempLeafNodes, 0, &currWidth, &wins, false, sharedData->sum_pc + current_leaf->value, sharedData->sum_user, sharedData);
		printf ("\nnode_value = %d\nwins = %d\ncurrWidth = %d\n\n", current_leaf->value, wins, currWidth);

		if ((float) wins / (float) currWidth > win_probability) {
			
			win_probability = (float) wins / (float) currWidth;
			win_node = sharedData->nodeMap->at (*it);
			max_sum = sharedData->sum_pc + current_leaf->value;
		
		} else if ((float) wins / (float) currWidth == win_probability) {
			
			if (max_sum < sharedData->sum_pc + current_leaf->value) {
				win_node = sharedData->nodeMap->at (*it);
				max_sum = sharedData->sum_pc + current_leaf->value;
			}
		}

		if (neighbor_of_current_leaf->neighbors.size () == 1) {

			tempLeafNodes.erase (neighbor_of_current_leaf);
		}

		neighbor_of_current_leaf->neighbors.insert (current_leaf);
		tempLeafNodes.insert (current_leaf);
	}

	if (sharedData->remainingNodes->size () == 1) {
		win_node = *sharedData->remainingNodes->begin ();
	}

    if (win_node != NULL) {
        win_node->status |= UNUSED;
        sharedData->sum_pc += win_node->value;
    }

    loadTreeVaosDFS (sharedData->root, sharedData);
    restoreBioDFS (sharedData->root);
	
	for (std::set <SimpleNode *>::iterator i = allSimpleNodes.begin (); i != allSimpleNodes.end (); ++i) {

		printf ("%d : ", (*i)->value);

		for (std::set <SimpleNode *>::iterator j = (*i)->neighbors.begin (); j != (*i)->neighbors.end (); ++j) {

			printf ("%d ", (*j)->value);
		}

		printf ("\n");
	}


	for (std::set <SimpleNode *>::iterator it = allSimpleNodes.begin (); it != allSimpleNodes.end (); ++it) {

		delete (*it);
	}
}


int getMaxWin (int it, int skip, int userTurnsLeft, int pcTurnsLeft, int userSum, int pcSum, SharedData* sharedData)
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

    if (it < (int) sharedData->usedTreeNodes->size ()) {

        if (pcTurnsLeft > 0) {
            pcSum += (*sharedData->usedTreeNodes)[it]->value;

            retMaxWin += getMaxWin (it + 1, skip, userTurnsLeft, pcTurnsLeft - 1, userSum, pcSum, sharedData);

            pcSum -= (*sharedData->usedTreeNodes)[it]->value;
        }

        if (userTurnsLeft > 0) {
            userSum += (*sharedData->usedTreeNodes)[it]->value;

            retMaxWin += getMaxWin (it + 1, skip, userTurnsLeft - 1, pcTurnsLeft, userSum, pcSum, sharedData);

            userSum -= (*sharedData->usedTreeNodes)[it]->value;
        }
    }

    return retMaxWin;
}


void computerTurn (SharedData* sharedData)
{
    sharedData->usedTreeNodes->clear ();

    getNodesDFS (sharedData->root, sharedData);
    restoreBioDFS (sharedData->root);

    int maxWin = 0, temp = 0, userTurns, pcTurns, maxValue = 0;
    Node* node = NULL;

    userTurns = (int) sharedData->usedTreeNodes->size () / 2;
    pcTurns = userTurns;

    if (userTurns + pcTurns < (int) sharedData->usedTreeNodes->size ()) {
        pcTurns++;
    }

    for (std::vector <Node*>::iterator it = sharedData->usedTreeNodes->begin ();
    it != sharedData->usedTreeNodes->end (); ++it) {

        if (!((*it)->status & ACTIVE)) {
            continue;
        }

        if (maxValue < (*it)->value) {
            maxValue = (*it)->value;
            node = *it;
        }
    }

    for (std::vector <Node*>::iterator it = sharedData->usedTreeNodes->begin ();
    it != sharedData->usedTreeNodes->end (); ++it) {

        if (!((*it)->status & ACTIVE)) {
            continue;
        }

        temp = getMaxWin (0, (int) (it - sharedData->usedTreeNodes->begin ()), userTurns, pcTurns, sharedData->sum_user, sharedData->sum_pc + (*it)->value, sharedData);

        printf ("%d\n", temp);

        if (maxWin < temp) {
            maxWin = temp;
            node = *it;
        }
    }

    printf ("done.\n");

    if (node != NULL) {
        node->status |= UNUSED;
        sharedData->sum_pc += node->value;
    }

    loadTreeVaosDFS (sharedData->root, sharedData);
    restoreBioDFS (sharedData->root);
}
