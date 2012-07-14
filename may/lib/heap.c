#include "heap.h"
#include "mem.h"
#include <assert.h>
#include <stdbool.h>

ERR_DEFINE(e_heap_invalid_size, "Heap size is less than you try to release.", 0);
ERR_DEFINE(e_heap_invalid_pointer, "Pointer referers to memory out of heap.", 0);

heap_t heap_create(size_t block_size) {
	heap_t res;
	if(block_size==0)
		block_size = 8*1024;
	res = mem_alloc(sizeof(heap_s) + block_size);
	res->block_size = block_size;
	res->last = &res->first;
	res->first.size = block_size;
	res->first.used = 0;
	res->first.next = 0;
	res->first.previous = 0;
	return res;
}

heap_t heap_delete(heap_t h) {
	if(h) {
		heap_block_t *p = h->first.next;
		while(p) {
			heap_block_t *tmp = p->next;
			mem_free(p);
			p = tmp;
		}
		mem_free(h);
	}
	return 0;
}

void *heap_slow_alloc(heap_t h, size_t sz) {
	size_t block_sz = sz>h->block_size ? sz : h->block_size;
	heap_block_t *b = h->last->next = mem_alloc(sizeof(heap_s) + block_sz);
	b->previous = h->last;
	h->last = b;
	b->size = block_sz;
	b->used = sz;
	b->next = 0;
	return &(b->data[0]);
}

void heap_release_to(heap_t h, void *p) {
	heap_block_t *i = h->last;
	while(i ? ((char *)p)<i->data || ((char *)p)>=(i->data+i->used) : false)
		i = i->previous;
	if(i) {
		heap_block_t *j;
		i->used = ((char *)p) - i->data;
		for(j=i->next; j; j=j->next)
			mem_free(j);
		i->next = 0;
	} else
		err_throw(e_heap_invalid_pointer);
}

void heap_clear(heap_t h) {
	heap_block_t *i;
	for(i = h->first.next; i; i=i->next)
		mem_free(i);
	h->first.next = 0;
	h->first.used = 0;
	h->last = &h->first;
}



