#ifndef APP_H__HEADER_GUARD__
#define APP_H__HEADER_GUARD__

#include <stdio.h>  /* puts, printf */
#include <stdlib.h> /* size_t */
#include <stdbool.h> /*bool, true, false */
#include <string.h> /* strcmp */
#include <assert.h> /* assert */

#include "error.h"
#include "vm.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0

typedef struct {
	char **argv;
	size_t argc;
} app_t;

void help(void);
void version(void);

void app_read_args(app_t *p_app, int p_argc, char **p_argv);
int  app_run_program(app_t *p_app);

#endif
