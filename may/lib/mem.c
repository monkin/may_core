#include "mem.h"
#include <stdlib.h>
#include <stdbool.h>

#ifdef MEM_CHECK
#include <execinfo.h>
#include <pthread.h>
#include <stdio.h>
	
typedef struct mem_check_item_ss {
	void *item;
	size_t item_size;
	void *stack[MEM_CHECK_STACK_SIZE];
	size_t stack_size;
	struct mem_check_item_ss *parent;
	struct mem_check_item_ss *children[2];
} mem_check_item_s;
typedef mem_check_item_s *mem_check_item_t;

pthread_mutex_t mem_check_mutex = PTHREAD_MUTEX_INITIALIZER;
mem_check_item_t mem_check_root = 0;
long long mem_check_alloc_count = 0;
long long mem_check_free_count = 0;
long long mem_check_allocated_size = 0;
long long mem_check_max_allocated_size = 0;
	
void mem_check_insert(void *ptr, size_t sz, mem_check_item_t removed_item) {
	mem_check_item_t item = removed_item ? removed_item : malloc(sizeof(mem_check_item_s));
	if(!item)
		err_throw(e_out_of_memory);
	item->item = ptr;
	item->item_size = sz;
	item->stack_size = backtrace(item->stack, MEM_CHECK_STACK_SIZE);
	item->children[0] = item->children[1] = 0;
	pthread_mutex_lock(&mem_check_mutex);
		if(!mem_check_root) {
			mem_check_root = item;
			item->parent = 0;
		} else {
			mem_check_item_t it = mem_check_root;
			while(true) {
				int index = ptr>it->item ? 1 : 0;
				if(!it->children[index]) {
					it->children[index] = item;
					item->parent = it;
					break;
				} else
					it = it->children[index];
			}
		}
	mem_check_alloc_count++;
	mem_check_allocated_size += sz;
	if(mem_check_allocated_size>mem_check_max_allocated_size)
		mem_check_max_allocated_size = mem_check_allocated_size;
	pthread_mutex_unlock(&mem_check_mutex);
}

mem_check_item_t mem_check_remove(void *ptr) {
	mem_check_item_t i;
	mem_check_item_t removed_item;
	pthread_mutex_lock(&mem_check_mutex);
	i = mem_check_root;
	while(i) {
		if(ptr==i->item) {
			if(i->children[0] && i->children[1]) {
				mem_check_item_t j = i->children[0];
				while(j->children[1])
					j = j->children[1];
				j->children[1] = i->children[1];
				j->children[1]->parent = j;
				i->children[1] = 0;
			}
			if(i==mem_check_root) {
				removed_item = mem_check_root;
				mem_check_root = i->children[i->children[0] ? 0 : 1];
			} else {
				int ci = i->parent->children[0]==i ? 0 : 1;
				i->parent->children[ci] = i->children[0] ? i->children[0] : i->children[1];
				if(i->parent->children[ci])
					i->parent->children[ci]->parent = i->parent; 
				removed_item = i;
			}
			break;
		} else
			i = i->children[ptr>i->item ? 1 : 0];
	}
	if(!removed_item) {
		pthread_mutex_unlock(&mem_check_mutex);
		err_throw(e_invalid_memory_block);
	} else {
		mem_check_free_count++;
		mem_check_allocated_size -= removed_item->item_size;
		pthread_mutex_unlock(&mem_check_mutex);
		return removed_item;
	}
}

void mem_check_log(mem_check_item_t i) {
	if(i) {
		fprintf(stderr, "Block %p %llu bytes at:\n ", i->item, (unsigned long long) i->item_size);
		backtrace_symbols_fd(i->stack, i->stack_size, STDERR_FILENO);
		mem_check_log(i->children[0]);
		mem_check_log(i->children[1]);
	}
}

void mem_check_close() {
	fprintf(stderr, "---\nMemory statistic\nUsed at exit %llu bytes in %llu blocks\n%llu mem_allocs, %llu mem_frees\nMax mem used %llu bytes\n---\n", mem_check_allocated_size, mem_check_alloc_count-mem_check_free_count, mem_check_alloc_count, mem_check_free_count, mem_check_max_allocated_size);
	mem_check_log(mem_check_root);
}

#endif /* MEM_CHECK */

ERR_DEFINE(e_out_of_memory, "Out of memory", 0);
ERR_DEFINE(e_invalid_memory_block, "Block shuld be previously allocated", 0);

void *mem_alloc(size_t sz) {
	void *res;
	err_reset();
	if(!sz)
		return 0;
	res = malloc(sz);
	if(!res)
		err_throw(e_out_of_memory);
#	ifdef MEM_CHECK
	err_try {
		mem_check_insert(res, sz, 0);
	} err_catch {
		free(res);
		err_throw_down();
	}
#	endif /* MEM_CHECK */
	return res;
}

void *mem_free(void *p) {
#	ifdef MEM_CHECK
	if(p)
		free(mem_check_remove(p));
#	endif
	free(p);
	return 0;
}

void *mem_realloc(void *p, size_t sz) {
	if(!sz)
		return mem_free(p);
	else if(!p) {
		return mem_alloc(sz);
	} else {
		void *res = realloc(p, sz);
		if(!res)
			err_throw(e_out_of_memory);
#	ifdef MEM_CHECK
		mem_check_insert(res, sz, mem_check_remove(p));
#	endif
		return res;
	}
}

void mem_init() {
#	ifdef MEM_CHECK
	atexit(mem_check_close);
#	endif
}

