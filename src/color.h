#ifndef COLORS_H__HEADER_GUARD__
#define COLORS_H__HEADER_GUARD__

#include <stdio.h>  /* FILE, fputs */
#include <assert.h> /* assert*/

#include "platform.h"

typedef enum {
	COLOR_DEFAULT = 0,

	COLOR_BLACK,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_YELLOW,
	COLOR_BLUE,
	COLOR_MAGENTA,
	COLOR_CYAN,
	COLOR_WHITE,

	COLOR_GREY,
	COLOR_BRIGHT_RED,
	COLOR_BRIGHT_GREEN,
	COLOR_BRIGHT_YELLOW,
	COLOR_BRIGHT_BLUE,
	COLOR_BRIGHT_MAGENTA,
	COLOR_BRIGHT_CYAN,
	COLOR_BRIGHT_WHITE,
} color_t;

void set_fg_color(color_t p_color, FILE *p_stream);

#endif
