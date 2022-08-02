#include "app.h"

/* TODO: make the arguments work more like in gcc */
int read_args(int p_argc, char **p_argv) {
	if (p_argc > 1) {
		if (strcmp(p_argv[1], "-h") == 0 || strcmp(p_argv[1], "--help") == 0) {
			if (p_argc > 2)
				fatal("Unexpected second argument '%s'", p_argv[2]);

			help();
		} else if (strcmp(p_argv[1], "-v") == 0 || strcmp(p_argv[1], "-version") == 0) {
			if (p_argc > 2)
				fatal("Unexpected second argument '%s'", p_argv[2]);

			version();
		} else
			return run_program(p_argc - 1, p_argv + 1);
	} else
		help();

	return 0;
}

void fatal(const char *p_fmt, ...) {
	char    msg[1024];
	va_list args;

	va_start(args, p_fmt);
	vsnprintf(msg, sizeof(msg), p_fmt, args);
	va_end(args);

	set_fg_color(COLOR_BRIGHT_RED);
	fputs("Error: ", stderr);

	set_fg_color(COLOR_DEFAULT);
	fprintf(stderr, "%s\nTry 'temple -h'\n", msg);

	exit(EXIT_FAILURE);
}

void help(void) {
	puts(" _..__ ________  _________ _      _____\n"
	     "|_|| _|  ___|  \\/\\_|| ___ \\ |    |  __/\n"
	     "  ||| | |__ | . \\._/| |,/ /\\\\    | |__ \n"
	     "  | |/|  _||| |\\/|_||  \\|/| \\|   |  _||\n"
	     " .| ||| |_``| | / _|| ||| | ||.._| |_||\n"
	     " \\\\_/\\\\____/\\_| ||_/\\_|   \\___||/\\____/\n"
	     " ||   |/        ||            //     ||\n"
	     "\n"
	     "Github: https://github.com/lordoftrident/temple\n"
	     "\n"
	     "Usage: temple [OPTIONS] [FILE [ARG...]]\n"
	     "OPTIONS:\n"
	     "  -h, --help,    - show this message\n"
	     "  -v, --version  - show the version");
}

void version(void) {
	printf("Temple %i.%i.%i\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}

int run_program(int p_argc, char **p_argv) {
	assert(p_argc > 0);

	inst_t  *program = NULL;
	header_t header  = read_program_file(&program, p_argv[0]);

	vm_t vm;
	vm_init(&vm);

	int ex = vm_exec(&vm, program, header.program_size, header.entry_point);

	free(program);

	return ex;
}

header_t read_program_file(inst_t **p_buff, const char *p_path) {
	assert(sizeof(word_t) == 8);

	FILE *file = fopen(p_path, "r");
	if (file == NULL)
		fatal("Could not open file '%s'", p_path);

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (size < 10)
		fatal("'%s' is in an incompatible file format (incomplete header)", p_path);

	char ch = fgetc(file);
	if (ch == '#')
		while (fgetc(file) != '\n'); /* skip the line */
	else
		ungetc(ch, file);

	char type[3] = {0};
	int tmp = fread(type, 1, sizeof(type) - 1, file);
	(void)tmp;

	if (strcmp(type, "TM") != 0)
		fatal("'%s' is in an incompatible file format (not TM format)", p_path);

	/* read the header */
	header_t header;

	/* the index of the instruction the program will start at */
	header.entry_point = file_read_word_bytes(file);

	/* program size is in instructions, not bytes */
	header.program_size = file_read_word_bytes(file);

	if (header.program_size > (size - HEADER_SIZE) / INST_SIZE)
		fatal("'%s' header has an incorrect program size", p_path);

	assert(INST_SIZE == 10);

	*p_buff = (inst_t*)malloc(header.program_size * sizeof(inst_t));
	if (p_buff == NULL)
		assert(0 && "malloc fail");

	for (size_t i = 0; i < header.program_size; ++ i) {
		uint8_t inst_bytes[INST_SIZE];

		size_t size;
		size = fread(inst_bytes, 1, sizeof(inst_bytes), file);
		if (size < sizeof(inst_bytes))
			fatal("'%s' has an incomplete instruction (probably not a compatible format)", p_path);

		(*p_buff)[i].opcode = inst_bytes[0];
		(*p_buff)[i].reg    = inst_bytes[1];
		(*p_buff)[i].data   = ((word_t)inst_bytes[2] << 070) |
		                      ((word_t)inst_bytes[3] << 060) |
		                      ((word_t)inst_bytes[4] << 050) |
		                      ((word_t)inst_bytes[5] << 040) |
		                      ((word_t)inst_bytes[6] << 030) |
		                      ((word_t)inst_bytes[7] << 020) |
		                      ((word_t)inst_bytes[8] << 010) |
		                      ((word_t)inst_bytes[9]);
	}

	fclose(file);

	return header;
}

word_t file_read_word_bytes(FILE *p_file) {
	uint8_t bytes[sizeof(word_t)];
	int tmp = fread(bytes, 1, sizeof(bytes), p_file);
	(void)tmp;

	return ((word_t)bytes[0] << 070) |
	       ((word_t)bytes[1] << 060) |
	       ((word_t)bytes[2] << 050) |
	       ((word_t)bytes[3] << 040) |
	       ((word_t)bytes[4] << 030) |
	       ((word_t)bytes[5] << 020) |
	       ((word_t)bytes[6] << 010) |
	        (word_t)bytes[7];
}
