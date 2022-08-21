#include "utils.h"

void *memalloc(size_t p_size) {
	void *ptr = malloc(p_size);
	if (ptr == NULL)
		assert(0 && "malloc fail");

	return ptr;
}

void *memrealloc(void *p_ptr, size_t p_size) {
	void *ptr = realloc(p_ptr, p_size);
	if (ptr == NULL) {
		free(p_ptr);

		assert(0 && "realloc fail");
	} else
		return ptr;
}

void memfree(void **p_ptr) {
	free(*p_ptr);
	*p_ptr = NULL;
}

bool fread64_little_endian(uint64_t *p_num, FILE *p_file) {
	uint8_t bytes[sizeof(uint64_t)];

	int tmp = fread(bytes, 1, sizeof(bytes), p_file);
	if (tmp < (int)sizeof(uint64_t))
		return false;

	*p_num = ((uint64_t)bytes[0] << 070) |
	         ((uint64_t)bytes[1] << 060) |
	         ((uint64_t)bytes[2] << 050) |
	         ((uint64_t)bytes[3] << 040) |
	         ((uint64_t)bytes[4] << 030) |
	         ((uint64_t)bytes[5] << 020) |
	         ((uint64_t)bytes[6] << 010) |
	          (uint64_t)bytes[7];

	return true;
}
