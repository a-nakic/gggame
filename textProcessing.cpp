#include "textProcessing.hpp"


void initText (SharedData* sharedData)
{
    FT_Library ft;
    if (FT_Init_FreeType (&ft)) {
        printf ("ERROR::FREETYPE: Could not init FreeType Library\n");
        return;
    }

    FT_Face face;
    if (FT_New_Face (ft, "res/fonts/OpenSans-Regular.ttf", 0, &face)) {
        printf ("ERROR::FREETYPE: Failed to load font\n");
        return;
    }
    FT_Set_Pixel_Sizes(face, 0, 48);

    for (unsigned char c = 0; c < 128; c++) {

        if (FT_Load_Char (face, c, FT_LOAD_RENDER)) {
            printf ("ERROR::FREETYTPE: Failed to load Glyph\n");
            continue;
        }

        glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

        unsigned int texture;
        glGenTextures (1, &texture);
        glBindTexture (GL_TEXTURE_2D, texture);
        glTexImage2D (
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glPixelStorei (GL_PACK_ALIGNMENT, 4);

        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            NULL,
            glm::fvec2 (face->glyph->bitmap.width / (double) sharedData->height, face->glyph->bitmap.rows / (double) sharedData->height),
            glm::fvec2 (face->glyph->bitmap_left / (double) sharedData->height, face->glyph->bitmap_top / (double) sharedData->height),
            (face->glyph->advance.x / (double) sharedData->height) / 64.0f
        };
        sharedData->characters->insert (std::pair<char, Character>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}


void renderText (TextObject* textObject)
{
    for (std::vector <Character>::iterator it = textObject->text->begin ();
    it != textObject->text->end (); ++it) {

        glBindTexture (GL_TEXTURE_2D, it->texture);
        glBindVertexArray (it->vao);
        glDrawElements (GL_TRIANGLES, 3*2, GL_UNSIGNED_INT, (void*)0);
    }
}

TextObject* loadTextObject (std::string* text, float x, float y, float scale, SharedData* sharedData)
{
	TextObject* ret = new TextObject;
	std::vector <Character>* vec = new std::vector <Character>;

	float max_x = -20.0f, max_y = -20.0f, min_x = 20.0f, min_y = 20.0f;

	for (std::string::iterator it = text->begin ();
	it != text->end (); ++it) {

		Character ch = (*sharedData->characters)[*it];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;

		max_x = std::max (max_x, xpos + w);
		max_y = std::max (max_y, ypos + h);
		min_x = std::min (min_x, xpos);
		min_y = std::min (min_y, ypos);

		Quad* textBox = new Quad;

		textBox->t4 = std::make_pair (xpos, ypos);
		textBox->t1 = std::make_pair (xpos, ypos + h);
		textBox->t2 = std::make_pair (xpos + w, ypos + h);
		textBox->t3 = std::make_pair (xpos + w, ypos);
		textBox->depth = -0.04f;

		GLuint* textVao = new GLuint;
		fillQuadBuffers (textVao, textBox, false);
		ch.vao = *textVao;

		vec->push_back (ch);

		delete textBox;
		delete textVao;

		x += ch.Advance * scale;
	}

    Quad* textBack = new Quad;

    textBack->t4 = std::make_pair (min_x - 0.01f, min_y - 0.01f);
    textBack->t1 = std::make_pair (min_x - 0.01f, max_y + 0.01f);
    textBack->t2 = std::make_pair (max_x + 0.01f, max_y + 0.01f);
    textBack->t3 = std::make_pair (max_x + 0.01f, min_y - 0.01f);
    textBack->depth = -0.039f;

    GLuint* backVao = new GLuint;
    fillQuadBuffers (backVao, textBack, true);

    ret->textBackVao = *backVao;
    ret->text = vec;

    return ret;
}


void deleteTextObject (TextObject* textObject)
{
    if (textObject != nullptr) {
        if (textObject->text != nullptr) {
            for (std::vector <Character>::iterator it = textObject->text->begin ();
            it != textObject->text->end (); ++it) {

                deleteQuadBuffers (&((*it).vao));
            }

            delete textObject->text;
        }

        delete textObject;
    }
}
