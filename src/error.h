#ifndef ERROR_H__HEADER_GUARD__
#define ERROR_H__HEADER_GUARD__

#include <stdarg.h> /* va_list, va_start, va_end, vsnprintf */
#include <stdio.h>  /* fputs, fprintf */
#include <stdlib.h> /* exit, EXIT_FAILURE */

#include "colors.h"

#define QUOTES(p_fmt) "\x1bW'"p_fmt"'\x1bX"

void fatal(const char *p_fmt, ...);

#endif
