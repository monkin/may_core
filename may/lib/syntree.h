#ifndef MAY_SYNTREE_H
#define MAY_SYNTREE_H

#include "str.h"
#include "heap.h"

typedef struct syntree_node_ss {
	int is_start;
	str_it_t position;
	heap_t heap;
	struct syntree_node_ss *next;
	int name;
	str_t value;
} syntree_node_s;

typedef syntree_node_s *syntree_node_t;

typedef struct syntree_ss {
	heap_t heap;
	syntree_node_t first;
	syntree_node_t last;
	struct syntree_ss *parent;
	str_t str;
	str_it_t position;
	str_it_t max_position;
} syntree_s;

typedef syntree_s *syntree_t;

syntree_t syntree_create(str_t);
syntree_t syntree_delete(syntree_t);

syntree_t syntree_transaction(syntree_t);
syntree_t syntree_commit(syntree_t);
syntree_t syntree_rollback(syntree_t);
void syntree_named_start(syntree_t, int);
void syntree_named_end(syntree_t);

#define syntree_position(st) ((st)->position)
#define syntree_str(st) ((st)->str)
void syntree_seek(syntree_t, str_it_t);
#define syntree_eof(st) ((st)->position==str_end(syntree_str(st)))

syntree_node_t syntree_begin(syntree_t);
syntree_node_t syntree_next(syntree_node_t);
syntree_node_t syntree_child(syntree_node_t);
/*int syntree_name(syntree_node_t);*/
#define syntree_name(stn) ((stn)->name)
str_t syntree_value(syntree_node_t);

size_t syntree_error_line(syntree_t);
size_t syntree_error_char(syntree_t);


#endif /* MAY_SYNTREE_H */
