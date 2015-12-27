#pragma once

#include <Windows.h>

void _DBGPRINT( LPCWSTR kwszFunction, INT iLineNumber, LPCWSTR kwszDebugFormatString, ... );
#ifdef _DEBUG
#define DBGPRINT(kwszDebugFormatString, ...) _DBGPRINT(__FUNCTIONW__, __LINE__, kwszDebugFormatString, __VA_ARGS__)
#else
#define DBGPRINT( kwszDebugFormatString, ... ) ;;
#endif

struct Buffer
{
	unsigned char * data;
	size_t width, height;
};
void copy_slice(const struct Buffer from, const struct Buffer to, size_t slice_width, size_t slice_height,
	size_t slice_from_x, size_t slice_from_y, size_t slice_to_x, size_t slice_to_y);