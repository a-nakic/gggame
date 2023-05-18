#ifndef TEXTPROCESSING_HPP
#define TEXTPROCESSING_HPP


#include "graphicAlgorithms.hpp"


void initText (SharedData* sharedData);


void renderText (TextObject* textObject);


TextObject* loadTextObject (std::string* text, float x, float y, float scale, SharedData* sharedData);


void deleteTextObject (TextObject* textObject);


#endif
