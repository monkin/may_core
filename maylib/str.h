#ifndef MAY_STR_H
#define MAY_STR_H

#include "err.h"
#include "heap.h"
#include <string.h>
#include <stdlib.h>

ERR_DECLARE(e_str_format);

typedef struct {
	size_t length;
	char *data;
} may_str_s;

typedef may_str_s *str_t;

typedef char *str_it_t;

str_t str_create(heap_t, size_t);

str_t str_from_cs(heap_t, const char *s);
str_t str_from_bin(heap_t, void *s, size_t sz);
str_t str_from_int(heap_t, long long);
str_t str_from_double(heap_t, double);

double str_to_double(str_t);
long long str_to_int(str_t);

str_t str_cat(heap_t, str_t, str_t);

/* str_it_t str_begin(str_t); */
#define str_begin(s) ((str_it_t)(s)->data)
str_it_t str_end(str_t);

/**
 * WARNING This function don't clone content of string. (use str_copy)
 * WARNING This function returns no zero-ended string. (use str_copy)
 */
str_t str_interval(heap_t, str_it_t, str_it_t);

str_t str_clone(heap_t, str_t);
int str_compare(str_t, str_t);
int str_equal(str_t, str_t);
/*size_t str_length(str_t);*/
#define str_length(s) ((s)->length)


typedef struct sbuilder_item_s {
	str_t data;
	struct sbuilder_item_s *next;
} sbuilder_item_t;

typedef struct {
	heap_t heap;
	sbuilder_item_t *first;
	sbuilder_item_t *last;
	size_t length;
} sbuilder_s;

typedef sbuilder_s *sbuilder_t;

/**
 * Something like Java StringBuilder
 * All appended strings must exist when sbuilder_get called.
 */
sbuilder_t sbuilder_create(heap_t h);
sbuilder_t sbuilder_append(sbuilder_t, str_t);
str_t sbuilder_get(heap_t h, sbuilder_t);



#endif /* MAY_STR_H */
