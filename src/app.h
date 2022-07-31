#ifndef APP_H__HEADER_GUARD__
#define APP_H__HEADER_GUARD__

#include <stdio.h>  /* puts, printf, fprintf, FILE, fopen, fclose, fseek, ftell, fgetc, fread */
#include <stdlib.h> /* exit, EXIT_FAILURE, malloc, free */
#include <string.h> /* strcmp */
#include <stdarg.h> /* va_list, va_start, va_end, vsnprintf */
#include <assert.h> /* assert */
#include <stdint.h> /* uint8_t, uint64_t */

#include "color.h"
#include "vm.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0

#define HEADER_SIZE 2 + sizeof(word_t) * 2

typedef struct {
	word_t entry_point, program_size;
} header_t;

int read_args(int p_argc, char **p_argv);

void fatal(const char *p_fmt, ...);

void help(void);
void version(void);
int  run_program(int p_argc, char **p_argv);

/* file format: TM <entry point instruction> <program size (in instructions)> <instructions> */
header_t read_program_file(inst_t **p_buff, const char *p_path);
word_t   file_read_word_bytes(FILE *p_file);

#endif
