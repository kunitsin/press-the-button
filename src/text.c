#include "text.h"

#include <Windows.h>
#include <tchar.h>

#include "freetype-2.5.5/include/ft2build.h"
#include FT_FREETYPE_H

#include "utils.h"

FT_Library library;
FT_Face face;

int init_text()
{
	if (FT_Init_FreeType(&library))
		return -1;

	if (FT_New_Face(library, "Raleway-ExtraLight.ttf", 0, &face))
		return -1;

	FT_Set_Pixel_Sizes(face, 0, 500);

	for (size_t i = 0; i < face->num_charmaps; i++)
	{
		if (face->charmaps[i]->encoding != FT_ENCODING_UNICODE)
			continue;

		if (face->charmap == face->charmaps[i])
			return 0;
		if (FT_Set_Charmap(face, face->charmaps[i]))
			return -2;
		return 0;
	}

	return -1;
}

int render_text(const char* text, GLuint* o_texture, unsigned int* o_width, unsigned int* o_height)
{
	struct Letter
	{
		FT_Int bitmap_left, bitmap_top;
		unsigned int width, rows;
		unsigned char * buffer;
	};

	const size_t chars = strlen(text);
	struct Letter * letters = malloc(sizeof(struct Letter) * chars);

	struct {int left, top, right, bottom;} Dimensions = {0};
	FT_Vector pen = {0, 0};
	for (size_t i = 0; i < chars; i++)
	{
		FT_Set_Transform(face, NULL, &pen);
		FT_Load_Char(face, text[i], FT_LOAD_RENDER);
		const FT_GlyphSlot slot = face->glyph;
		DBGPRINT(L"%c bitmap = %u %u %u %u advance = %u %u\n", text[i], slot->bitmap_left, slot->bitmap_top,
			slot->bitmap.width, slot->bitmap.rows, slot->advance.x, slot->advance.y);

		const size_t bufsize = sizeof(unsigned char) * slot->bitmap.width * slot->bitmap.rows;
		letters[i] = (struct Letter) {
			slot->bitmap_left,
			slot->bitmap_top,
			slot->bitmap.width,
			slot->bitmap.rows,
			malloc(bufsize)
		};
		memcpy(letters[i].buffer, slot->bitmap.buffer, bufsize);

		pen.x += slot->advance.x;
		pen.y += slot->advance.y;

		if (slot->bitmap_left < Dimensions.left)
			Dimensions.left = slot->bitmap_left;
		if (slot->bitmap_top < Dimensions.top)
			Dimensions.top = slot->bitmap_top;
		if (slot->bitmap_left + slot->bitmap.width > Dimensions.right)
			Dimensions.right = slot->bitmap_left + slot->bitmap.width;
		if (slot->bitmap_top + slot->bitmap.rows > Dimensions.bottom)
			Dimensions.bottom = slot->bitmap_top + slot->bitmap.rows;
	}
	DBGPRINT(L"dims = %u %u %u %u\n", Dimensions.left, Dimensions.top, Dimensions.right, Dimensions.bottom);

	*o_width  = Dimensions.right  - Dimensions.left;
	*o_height = Dimensions.bottom - Dimensions.top;
	unsigned char * texture = malloc(sizeof(char) * (*o_width) * (*o_height));
	memset(texture, 0, sizeof(char) * (*o_width) * (*o_height));
	for (size_t i = 0; i < chars; i++)
	{
		struct Buffer from = {letters[i].buffer, letters[i].width, letters[i].rows};
		struct Buffer to = {texture, *o_width, *o_height};
		copy_slice(from, to, letters[i].width, letters[i].rows, 0, 0, letters[i].bitmap_left, letters[i].bitmap_top);
	}

	glGenTextures(1, o_texture);
	glBindTexture(GL_TEXTURE_2D, *o_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* Disable alignment in glTexImage2D */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, *o_width, *o_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, texture);

	for (size_t i = 0; i < chars; i++)
	{
		free(letters[i].buffer);
	}
	free(letters);
	free(texture);
	
	return glGetError() == GL_NO_ERROR ? 0 : -1;
}

void cleanup_text()
{
	FT_Done_Face(face);
	FT_Done_FreeType(library);
}