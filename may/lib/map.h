
#ifndef MAY_MAP_H
#define MAY_MAP_H

#include "str.h"
#include "heap.h"

typedef struct map_node_ss {
	str_t key;
	void *value;
	size_t length; /* count(descendant-or-self::*) */
	struct map_node_ss *children[2];
	struct map_node_ss *parent;
} map_node_s;

typedef map_node_s *map_node_t;

typedef struct map_ss {
	heap_t heap;
	size_t length;
	map_node_t node;
} map_s;

typedef map_s *map_t;

map_t map_create(heap_t h);
map_t map_optimize(map_t);
map_t map_set(map_t, str_t key, void *value);
map_t map_set_cs(map_t, const char *key, void *value);
map_t map_set_bin(map_t, const void *key, size_t key_len, void *value);
map_node_t map_find(map_t, str_t);
map_node_t map_find_cs(map_t, const char *);
map_node_t map_find_bin(map_t, const void *key, size_t key_len);
void *map_get(map_t, str_t key);
void *map_get_cs(map_t, const char *key);
void *map_get_bin(map_t, const void *key, size_t key_len);
void map_remove(map_t, str_t);
void map_remove_cs(map_t, const char *);
void map_remove_bin(map_t, const void *key, size_t key_len);
map_node_t map_remove_node(map_t, map_node_t); /* allways return NULL */
map_node_t map_begin(map_t);
map_node_t map_next(map_node_t);
#define map_length(m) ((m)->length)


#endif /* MAY_MAP_H */

