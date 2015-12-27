#include "utils.h"
#include <malloc.h>
#include <Strsafe.h>
#include <assert.h>

void _DBGPRINT( LPCWSTR kwszFunction, INT iLineNumber, LPCWSTR kwszDebugFormatString, ... )
{
    INT cbFormatString = 0;
    va_list args;
    PWCHAR wszDebugString = NULL;
    size_t st_Offset = 0;

    va_start( args, kwszDebugFormatString );

    cbFormatString = _scwprintf( L"[%s:%d] ", kwszFunction, iLineNumber ) * sizeof( WCHAR );
    cbFormatString += _vscwprintf( kwszDebugFormatString, args ) * sizeof( WCHAR ) + 2;

    /* Depending on the size of the format string, allocate space on the stack or the heap. */
    wszDebugString = (PWCHAR)_malloca( cbFormatString );

    /* Populate the buffer with the contents of the format string. */
    StringCbPrintfW( wszDebugString, cbFormatString, L"[%s:%d] ", kwszFunction, iLineNumber );
    StringCbLengthW( wszDebugString, cbFormatString, &st_Offset );
    StringCbVPrintfW( &wszDebugString[st_Offset / sizeof(WCHAR)], cbFormatString - st_Offset, kwszDebugFormatString, args );

    OutputDebugStringW( wszDebugString );

    _freea( wszDebugString );
    va_end( args );
}

void copy_slice(const struct Buffer from, const struct Buffer to, size_t slice_width, size_t slice_height,
	size_t slice_from_x, size_t slice_from_y, size_t slice_to_x, size_t slice_to_y)
{
	assert(slice_from_x + slice_width  <= from.width);
	assert(slice_from_y + slice_height <= from.height);
	assert(slice_to_x   + slice_width  <= to.width);
	assert(slice_to_y   + slice_height <= to.height);

	size_t from_line_start = slice_from_y * from.width + slice_from_x;
	size_t to_line_start   = slice_to_y   * to.width   + slice_to_x;
	for (size_t i = 0; i < slice_height; i++)
	{
		memcpy(to.data + to_line_start, from.data + from_line_start, slice_width);

		from_line_start += from.width;
		to_line_start   += to.width;
	}
}