
#ifndef MAY_PARSER_H
#define MAY_PARSER_H

#include "heap.h"
#include "map.h"
#include "str.h"
#include "syntree.h"
#include <stdbool.h>


struct parser_s {
	bool (*fn)(syntree_t, void *);
	void *data;
};

typedef struct parser_s *parser_t;


syntree_t parser_process(heap_t, parser_t, str_t);

parser_t parser_crange(heap_t, char, char);
parser_t parser_cset(heap_t, const char *);
parser_t parser_string(heap_t, const char *);
parser_t parser_predicate(heap_t, bool (*p)(char));
parser_t parser_fn(heap_t, bool (*fn)(syntree_t, void *), void *);
parser_t parser_or(heap_t, parser_t, parser_t);
parser_t parser_and(heap_t, parser_t, parser_t);
parser_t parser_rep(heap_t, parser_t, size_t, size_t);
parser_t parser_maybe(heap_t, parser_t);
parser_t parser_named(heap_t, int, parser_t);

#endif /* MAY_PARSER_H */

