/*
 * memory.h - the checked storage allocator in memory.c
 */

#ifndef memory_h_included
#define memory_h_included

#include <stddef.h>

extern void mem_debug(const int level);
extern void mem_tracking(int flag);
extern int mem_check(char *fname, int linenum);
extern void *mem_malloc(const size_t bytes);
extern void mem_free(const void *ptr);
extern void *mem_realloc(const void *ptr, const size_t new_size);
extern int mem_validate(const void *ptr);

#endif
