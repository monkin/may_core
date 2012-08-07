#ifndef MAY_MEM_H
#define MAY_MEM_H

#include "err.h"
#include <stddef.h>

#define MEM_CHECK
#define MEM_CHECK_STACK_SIZE 10

ERR_DECLARE(e_out_of_memory);
ERR_DECLARE(e_invalid_memory_block);

void mem_init();
void *mem_alloc(size_t);
void *mem_free(void *);
void *mem_realloc(void *, size_t);

#endif /* MAY_MEM_H */
