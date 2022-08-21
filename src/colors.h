#ifndef COLORS_H__HEADER_GUARD__
#define COLORS_H__HEADER_GUARD__

#include <stdio.h>   /* stdout, fputs, fflush, fprintf, fputc, FILE */
#include <stdarg.h>  /* va_list, va_start, va_end, vsnprintf, fputs */
#include <stdbool.h> /* bool, true, false */

#include "utils.h"

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
	COLOR_BRIGHT_WHITE
} color_t;

void set_fg_color(color_t p_color);
void fprintclrf(FILE *p_stream, const char *p_fmt, ...);

/*
	fprintclrf color formatting (\e = escape):
		\eX - default color

		\eo - black
		\er - red
		\eg - green
		\ey - yellow
		\el - blue
		\em - magenta
		\en - cyan
		\ew - white

		\eO - grey
		\eR - bright red
		\eG - bright green
		\eY - bright yellow
		\eL - bright blue
		\eM - bright magenta
		\eN - bright cyan
		\eW - bright white
*/

#endif
