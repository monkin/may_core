
#ifndef MAY_TAR_H
#define MAY_TAR_H

#include "stream.h"
#include "str.h"
#include "map.h"

ERR_DECLARE(e_tar_error);
ERR_DECLARE(e_tar_name_too_long);
ERR_DECLARE(e_tar_file_too_large);
ERR_DECLARE(e_tar_invalid_checksum);

struct tar_ss;
typedef struct tar_ss tar_s;
typedef tar_s *tar_t;

typedef struct tar_item_ss {
	tar_t tar;
	str_t name;
	long long position;
	long long size;
	ios_t range;
	struct tar_item_ss *next;
} tar_item_s;

typedef tar_item_s *tar_item_t;
typedef tar_item_t tar_it_t;

struct tar_ss {
	heap_t heap;
	ios_t stream;
	map_t files;
	tar_item_t first;
	tar_item_t last;
};

tar_t tar_create(ios_t);
ios_t tar_find(tar_t, str_t fname);
str_t tar_get(tar_t, heap_t, str_t fname);
void tar_put(tar_t, str_t fname, str_t content);
void tar_putf(tar_t, str_t fname, str_t path);
void tar_puts(tar_t, str_t fname, ios_t);
tar_t tar_delete(tar_t);

/* tar_item_t tar_it_first(tar_t); */
#define tar_it_first(t) ((t)->first)
/* tar_item_t tar_it_next(tar_item_t); */
#define tar_it_next(ti) ((ti)->next)

#define tar_it_name(ti) ((ti)->name)
#define tar_it_size(ti) ((ti)->size)
#define tar_it_stream(ti) ((ti)->range)


#endif /* MAY_TAR_H */

