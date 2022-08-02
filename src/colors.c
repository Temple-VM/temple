#include "colors.h"

void set_fg_color(color_t p_color) {
#ifdef PLATFORM_LINUX
	switch (p_color) {
	case COLOR_DEFAULT: fputs("\x1b[0m",  stdout); break;
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
	default: assert(0 && "Invalid color, this should never show");
	}
#endif
}
