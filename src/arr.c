
#include "arr.h"
#include <assert.h>
#include <string.h>

arr_t arr_create(heap_t h, size_t item_size, size_t length) {
	arr_t r = heap_alloc(h, sizeof(arr_s) + (item_size * length));
	r->item_size = item_size;
	r->length = length;
	r->data = ((char *) r) + sizeof(arr_s);
	return r;
}

arr_t arr_free(arr_t arr) {
	if(arr) {
		heap_free(arr->data);
		heap_free(arr);
	}
	return 0;
}

arr_t arr_concat(heap_t h, arr_t a1, arr_t a2) {
	assert(a1->item_size==a2->item_size);
	arr_t r = arr_create(h, a1->item_size, a1->length + a2->length);
	memcpy(r->data, a1->data, a1->item_size * a1->length);
	memcpy(((char *)r->data) + (a1->length * a1->item_size), a2->data, a2->length * a2->item_size);
	return r;
}

arr_t arr_slice(heap_t h, arr_t arr, size_t i1, size_t i2) {
	assert(i2>=i1);
	assert(i1 < arr->length && i2 < arr->length);
	arr_t r = heap_alloc(h, sizeof(arr_s));
	r->length = i2 - i1;
	r->item_size = arr->item_size;
	r->data = ((char *) arr->data) + (r->item_size * i1);
	return r;
}

arr_t arr_clone(heap_t h, arr_t arr) {
	arr_t r = arr_create(h, arr->item_size, arr->length);
	memcply(r->data, arr->data, r->item_size * r->length); 
	return r;
}