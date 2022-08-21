#ifndef UTILS_H__HEADER_GUARD__
#define UTILS_H__HEADER_GUARD__

#include <stdio.h>   /* FILE, fread */
#include <assert.h>  /* assert */
#include <stdlib.h>  /* size_t, malloc, realloc, free */
#include <stdbool.h> /* bool, true, false */
#include <stdint.h>  /* uint8_t, uint64_t */

#define INTERNAL_BUG \
	assert(0 && "If this shows, it is an internal bug, please report it")

#define SMEMALLOC(p_ptr, p_size)   (p_ptr = memalloc(sizeof(p_ptr[0]) * p_size))
#define SMEMREALLOC(p_ptr, p_size) (p_ptr = memrealloc(p_ptr, sizeof(p_ptr[0]) * p_size))
#define SMEMFREE(p_ptr)            (memfree((void**)&p_ptr))

#define SIZE_OF(p_ptr) (sizeof(p_ptr) / sizeof(p_ptr[0]))

void *memalloc(size_t p_size);
void *memrealloc(void *p_ptr, size_t p_size);
void  memfree(void **p_ptr);

bool fread64_little_endian(uint64_t *p_num, FILE *p_file);

#endif
