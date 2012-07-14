
#ifndef MAY_MCL_FILTER_CONTEXT_H
#define MAY_MCL_FILTER_CONTEXT_H

#include "ex.h"
#include "mcl.h"
#include "../lib/json.h"
#include "../lib/map.h"
#include "../lib/floader.h"
#include "../lib/mutex.h"
#include <stdbool.h>
#include <CL/cl.h>

typedef struct {
	heap_t heap;
	mutex_t mutex;
	cl_context opencl_context;
	floader_t file_loader;
	map_t objects;
} flcontext_s;

typedef flcontext_s *flcontext_t;

flcontext_t flcontext_create();
flcontext_t flcontext_delete(flcontext_t);

cl_mem flcontext_lock_object(str_t); /* lock and return memory object */
cl_mem flcontext_unlock_object(str_t); /* unlock object and return NULL */
cl_mem flcontext_set_object(str_t, cl_mem);
cl_mem flcontext_retain_object(cl_mem); /* increment reference counter */
cl_mem flcontext_release_object(cl_mem); /*  decrement reference counter */

#endif /* MAY_MCL_FILTER_CONTEXT_H */

