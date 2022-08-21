#include "app.h"

void help(void) {
	puts(" _..__ ________  _________ _      _____\n"
		"|_|| _|  ___|  \\/\\_|| ___ \\ |    |  __/\n"
		"  ||| | |__ | . \\._/| |,/ /\\\\    | |__ \n"
		"  | |/|  _||| |\\/|_||  \\|/| \\|   |  _||\n"
		" .| ||| |_``| | / _|| ||| | ||.._| |_||\n"
		" \\\\_/\\\\____/\\_| ||_/\\_|   \\___||/\\____/\n"
		" ||   |/        ||            //     ||\n"
		"\n"
		"Github: https://github.com/Temple-VM/temple\n"
		"\n"
		"Usage: temple [OPTIONS] [FILE [ARG...]]\n"
		"OPTIONS:\n"
		"  -h, --help,    - show this message\n"
		"  -v, --version  - show the version");

	exit(0);
}

void version(void) {
	printf("Temple %i.%i.%i\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

	exit(0);
}

void app_read_args(app_t *p_app, int p_argc, char **p_argv) {
	if (p_argc <= 1)
		help();

	p_app->argv = p_argv;
	p_app->argc = p_argc;

	for (size_t i = 1; i < (size_t)p_argc; ++ i) {
		if (strcmp(p_argv[i], "-h") == 0 || strcmp(p_argv[i], "--help") == 0)
			help();
		else if (strcmp(p_argv[i], "-v") == 0 || strcmp(p_argv[i], "--version") == 0)
			version();
		else {
			p_app->argv += i;
			p_app->argc -= i;

			return;
		}
	}

	/* this will only be reached when no file name was found */
	fatal("No input file");
}

int app_run_program(app_t *p_app) {
	assert(p_app->argc > 0);

	vm_t vm;
	vm_init(&vm);
	vm_load_from_file(&vm, p_app->argv[0]);

	return vm_exec(&vm);
}
