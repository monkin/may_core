
#ifndef MAY_MCL_ILOADER_H
#define MAY_MCL_ILOADER_H

#include "../lib/floader.h"
#include "../lib/map.h"
#include "image.h"

struct iloader_ss;
typedef struct iloader_ss iloader_s;
typedef iloader_s *iloader_t;

typedef struct iloader_node_ss {
	heap_t heap;
	str_t name;
	cl_mem image;
	iloader_t iloader;
	struct iloader_node_ss *parent;
	struct iloader_node_ss *childeren[2];
} iloader_node_s;

typedef iloader_node_s *iloader_node_t;

struct iloader_ss {
	heap_t heap;
	cl_context context;
	floader_t file_loader;
	iloader_node_t node;
} iloader_s;

iloader_t iloader_create(floader_t fl);
iloader_t iloader_delete(floader_t fl);
cl_mem iloader_load(iloader_t, str_t);
cl_mem iloader_load_cs(iloader_t, const char *);
cl_mem iloader_unload(iloader_t, str_t);
cl_mem iloader_unload_cs(iloader_t, const char *);
cl_mem iloader_unload_clmem(iloader_t, cl_mem);


#endif /* MAY_MCL_ILOADER_H */
