#include "colors.h"

void set_fg_color(color_t p_color) {
	switch (p_color) {
	case COLOR_DEFAULT: fputs("\x1b[0m",    stdout); break;
	case COLOR_BLACK:   fputs("\x1b[0;30m", stdout); break;
	case COLOR_RED:     fputs("\x1b[0;31m", stdout); break;
	case COLOR_GREEN:   fputs("\x1b[0;32m", stdout); break;
	case COLOR_YELLOW:  fputs("\x1b[0;33m", stdout); break;
	case COLOR_BLUE:    fputs("\x1b[0;34m", stdout); break;
	case COLOR_MAGENTA: fputs("\x1b[0;35m", stdout); break;
	case COLOR_CYAN:    fputs("\x1b[0;36m", stdout); break;
	case COLOR_WHITE:   fputs("\x1b[0;37m", stdout); break;

	case COLOR_GREY:           fputs("\x1b[1;90m", stdout); break;
	case COLOR_BRIGHT_RED:     fputs("\x1b[1;91m", stdout); break;
	case COLOR_BRIGHT_GREEN:   fputs("\x1b[1;92m", stdout); break;
	case COLOR_BRIGHT_YELLOW:  fputs("\x1b[1;93m", stdout); break;
	case COLOR_BRIGHT_BLUE:    fputs("\x1b[1;94m", stdout); break;
	case COLOR_BRIGHT_MAGENTA: fputs("\x1b[1;95m", stdout); break;
	case COLOR_BRIGHT_CYAN:    fputs("\x1b[1;96m", stdout); break;
	case COLOR_BRIGHT_WHITE:   fputs("\x1b[1;97m", stdout); break;

	default: INTERNAL_BUG;
	}

	fflush(stdout);
}

void fprintclrf(FILE *p_stream, const char *p_fmt, ...) {
	char    msg[1024];
	va_list args;

	va_start(args, p_fmt);
	vsnprintf(msg, sizeof(msg), p_fmt, args);
	va_end(args);

	bool escape = false;
	for (char *ch = msg; *ch != '\0'; ++ ch) {
		if (escape) {
			escape = false;

			switch (*ch) {
			case 'X': set_fg_color(COLOR_DEFAULT); break;

			case 'o': set_fg_color(COLOR_BLACK);   break;
			case 'r': set_fg_color(COLOR_RED);     break;
			case 'g': set_fg_color(COLOR_GREEN);   break;
			case 'y': set_fg_color(COLOR_YELLOW);  break;
			case 'l': set_fg_color(COLOR_BLUE);    break;
			case 'm': set_fg_color(COLOR_MAGENTA); break;
			case 'n': set_fg_color(COLOR_CYAN);    break;
			case 'w': set_fg_color(COLOR_WHITE);   break;

			case 'O': set_fg_color(COLOR_GREY);           break;
			case 'R': set_fg_color(COLOR_BRIGHT_RED);     break;
			case 'G': set_fg_color(COLOR_BRIGHT_GREEN);   break;
			case 'Y': set_fg_color(COLOR_BRIGHT_YELLOW);  break;
			case 'L': set_fg_color(COLOR_BRIGHT_BLUE);    break;
			case 'M': set_fg_color(COLOR_BRIGHT_MAGENTA); break;
			case 'N': set_fg_color(COLOR_BRIGHT_CYAN);    break;
			case 'W': set_fg_color(COLOR_BRIGHT_WHITE);   break;

			default:
				fputc('\x1b', p_stream);
				fputc(*ch,    p_stream);
			}

			continue;
		}

		if (*ch == '\x1b')
			escape = true;
		else
			fputc(*ch, p_stream);
	}
}
