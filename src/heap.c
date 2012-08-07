#include "heap.h"
#include "mem.h"
#include <assert.h>
#include <stdbool.h>

ERR_DEFINE(e_heap_invalid_size, "Heap size is less than you try to release.", 0);
ERR_DEFINE(e_heap_invalid_pointer, "Pointer referers to memory out of heap.", 0);

static heap_block_t heap_create_block(heap_t h, size_t sz) {
	heap_block_t r = mem_alloc(sizeof(heap_block_s) + sz - 1);
	r->size = sz;
	r->used = 0;
	r->count = 0;
	r->heap = h;
	return r;
}

#define heap_delete_block(b) (mem_free(b))

heap_t heap_create(size_t block_size) {
	heap_t res;
	if(block_size==0)
		block_size = HEAP_DEFAULT_BLOCK_SIZE;
	res = mem_alloc(sizeof(heap_s));
	res->block_size = block_size;
	res->first = res->last = 0;
	return res;
}

heap_t heap_delete(heap_t h) {
	if(h) {
		heap_block_t p = h->first;
		while(p) {
			heap_block_t tmp = p->next;
			mem_free(p);
			p = tmp;
		}
		mem_free(h);
	}
	return 0;
}

void *heap_alloc(heap_t h, size_t sz) {
	size_t necessary_size = sz + sizeof(heap_block_t);
	heap_block_t hb = h->last;
	if(!hb ? true : (necessary_size > (hb->size - hb->used))) {
		hb = heap_create_block(h, necessary_size>(h->block_size/2) ? necessary_size : h->block_size);
		if(!h->first) {
			h->first = h->last = hb;
			hb->next = hb->previous = 0;
		} else if(hb->size==necessary_size) {
			hb->next = h->first;
			hb->previous = 0;
			h->first->previous = hb;
			h->first = hb;
		} else {
			hb->previous = h->last;
			hb->next = 0;
			h->last->next = hb;
			h->last = hb;
		}
	}
	void *r = ((char *) hb->data) + hb->used;
	hb->count++;
	hb->used += necessary_size;
	*((heap_block_t *) r) = hb;
	return ((char *) r) + sizeof(heap_block_t);
}

void *heap_free(void *p) {
	if(p) {
		heap_block_t block = *((heap_block_t *)(((char *) p) - sizeof(heap_block_t)));
		assert(block->count);
		block->count--;
		if(!block->count) {
			if(!block->next && block->size==block->heap->block_size)
				block->used = 0;
			else {
				if(block->next)
					block->next->previous = block->previous;
				else
					block->heap->last = block->previous;
				if(block->previous)
					block->previous->next = block->next;
				else
					block->heap->first = block->next;
				heap_delete_block(block);
			}
		}
	}
	return 0;
}

