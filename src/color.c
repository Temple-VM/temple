#include "color.h"

void set_fg_color(color_t p_color, FILE *p_stream) {
#ifdef PLATFORM_LINUX
	switch (p_color) {
	case COLOR_DEFAULT: fputs("\x1b[0m",  p_stream); break;
	case COLOR_BLACK:   fputs("\x1b[0;30m", p_stream); break;
	case COLOR_RED:     fputs("\x1b[0;31m", p_stream); break;
	case COLOR_GREEN:   fputs("\x1b[0;32m", p_stream); break;
	case COLOR_YELLOW:  fputs("\x1b[0;33m", p_stream); break;
	case COLOR_BLUE:    fputs("\x1b[0;34m", p_stream); break;
	case COLOR_MAGENTA: fputs("\x1b[0;35m", p_stream); break;
	case COLOR_CYAN:    fputs("\x1b[0;36m", p_stream); break;
	case COLOR_WHITE:   fputs("\x1b[0;37m", p_stream); break;

	case COLOR_GREY:           fputs("\x1b[1;90m", p_stream); break;
	case COLOR_BRIGHT_RED:     fputs("\x1b[1;91m", p_stream); break;
	case COLOR_BRIGHT_GREEN:   fputs("\x1b[1;92m", p_stream); break;
	case COLOR_BRIGHT_YELLOW:  fputs("\x1b[1;93m", p_stream); break;
	case COLOR_BRIGHT_BLUE:    fputs("\x1b[1;94m", p_stream); break;
	case COLOR_BRIGHT_MAGENTA: fputs("\x1b[1;95m", p_stream); break;
	case COLOR_BRIGHT_CYAN:    fputs("\x1b[1;96m", p_stream); break;
	case COLOR_BRIGHT_WHITE:   fputs("\x1b[1;97m", p_stream); break;
	default: assert(0 && "Invalid color, this should never show");
	}
#endif
}
