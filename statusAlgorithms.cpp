#include "statusAlgorithms.hpp"


void updateStandings (SharedData* sharedData)
{
	int pot = 10000;
	std::string s1 ("user"), s2 ("pc"), s_level ("level : ");;
	std::string s3, s4;

	s_level.push_back (sharedData->level + '1');
	s_level.append ("/3");
	std::cout << s_level << std::endl;

	for (int i = 0; i < 4; i++) {
		char c1 = (char) (sharedData->sum_user % pot / (pot / 10) + '0');
		char c2 = (char) (sharedData->sum_pc % pot / (pot / 10) + '0');
		pot /= 10;

		s3.push_back (c1);
		s4.push_back (c2);
	}

	for (std::vector <TextObject*>::iterator it = sharedData->standings->begin ();
	it != sharedData->standings->end (); ++it) {

		deleteTextObject (*it);
	}
	sharedData->standings->clear ();

	sharedData->standings->push_back (loadTextObject (&s1, -sharedData->inv_asp, 0.1f, 1.0f, sharedData));
	sharedData->standings->push_back (loadTextObject (&s3, -sharedData->inv_asp, 0.05f, 1.0f, sharedData));
	sharedData->standings->push_back (loadTextObject (&s2, -sharedData->inv_asp, -0.05f, 1.0f, sharedData));
	sharedData->standings->push_back (loadTextObject (&s4, -sharedData->inv_asp, -0.1f, 1.0f, sharedData));
	sharedData->standings->push_back (loadTextObject (&s_level, -sharedData->inv_asp, -0.2f, 1.0f, sharedData));
}
