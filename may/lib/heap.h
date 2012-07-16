#ifndef MAY_HEAP_H
#define MAY_HEAP_H

#include "mem.h"
#include <stddef.h>
#include <stdio.h>

ERR_DECLARE(e_heap_invalid_size);
ERR_DECLARE(e_heap_invalid_pointer);

enum {
	HEAP_DEFAULT_BLOCK_SIZE = 8*1024
}

typedef struct heap_block_s {
	size_t size;
	size_t used;
	size_t count;
	struct heap_block_s *next;
	struct heap_block_s *previous;
	char data[1];
} heap_block_t;

typedef struct {
	size_t block_size;
	heap_block_t *last;
	heap_block_t *first;
} heap_s;

typedef heap_s *heap_t;

heap_t heap_create(size_t block_size);
heap_t heap_delete(heap_t);

void *heap_alloc(heap_t, size_t);


#endif /* MAY_HEAP_H */



