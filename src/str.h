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
#define str_free(s) ((str_t) heap_free(s))

str_t str_from_cs(heap_t, const char *s);
str_t str_from_bin(heap_t, const void *s, size_t sz);
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
int str_compare_cs(str_t, const char *);
int str_compare_bin(str_t, const void *, size_t);
#define str_cmp str_compare
#define str_cmp_cs str_compare_cs
#define str_cmp_bin str_compare_bin
int str_equal(str_t, str_t);
/*size_t str_length(str_t);*/
#define str_length(s) ((s)->length)


/**
 * Something like Java StringBuilder
 * All appended strings must exist when sb_get called.
 */
struct sb_ss;
typedef struct sb_ss sb_s;
typedef sb_s *sb_t;

typedef struct sb_item_ss {
	str_t str;
	sb_t sb;
	struct sb_item_ss *next;
} sb_item_s;

typedef sb_item_s *sb_item_t;

struct sb_ss {
	heap_t heap;
	sb_item_t first;
	sb_item_t last;
	size_t length;
};

sb_t sb_create(heap_t h);
sb_t sb_append(sb_t, str_t);
/* sb_t sb_append_cs(sb_t, const char *); */
#define sb_append_cs(sb, cs) sb_append((sb), str_from_cs(sb->heap, (cs)));
sb_t sb_append_sb(sb_t, sb_t);
sb_t sb_preppend(sb_t, str_t);
/* sb_t sb_preppend_cs(sb_t, const char *); */
#define sb_preppend_cs(sb, cs) sb_preppend((sb), str_from_cs(sb->heap, (cs)));
sb_t sb_preppend_sb(sb_t, sb_t);
str_t sb_get(heap_t h, sb_t);



#endif /* MAY_STR_H */
