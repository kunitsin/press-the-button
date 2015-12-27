#pragma once

#include "glew.h"

int init_text();
int render_text(const char* text, GLuint* o_texture, unsigned int* o_width, unsigned int* o_height);