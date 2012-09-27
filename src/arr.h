
#ifndef MAY_ARR_H
#define MAY_ARR_H

#include "heap.h"

typedef struct {
	size_t item_size;
	size_t length;
	void *data;
} arr_s;

typedef arr_s *arr_t;

arr_t arr_create(heap_t h, size_t item_size, size_t length);
arr_t arr_free(arr_t);

#define arr_length(arr) ((arr)->length)
#define arr_item_size(arr) ((arr)->item_size)


/**
 * for(my_type *i = arr_begin(a); i!=arr_end(a); i++) {
 *     // do something
 * }
 */
#define arr_begin(arr) ((arr)->data)
void *arr_end(arr_t);

arr_t arr_concat(heap_t, arr_t, arr_t);
arr_t arr_slice(heap_t, arr_t, size_t, size_t);

#endif /* MAY_ARR_H */
