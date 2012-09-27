
#include "arr.h"

arr_t arr_create(heap_t h, size_t item_size, size_t length) {
	arr_t r = heap_alloc(h, sizeof(arr_s));
	r->item_size = item_size;
	r->length = length;
	r->data = heap_alloc(h, item_size*length);
	return r;
}

arr_t arr_free(arr_t arr) {
	if(arr) {
		heap_free(arr->data);
		heap_free(arr);
	}
	return 0;
}