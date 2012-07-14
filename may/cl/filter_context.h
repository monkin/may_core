
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

typedef struct {
	mutex_t mutex;
	size_t ref_count;
	void *value;
	void (*destruction_callback)(void *, void *);
	void destruction_callback_data;
} flcontext_object_s;

typedef flcontext_object_s *flcontext_object_t;

flcontext_t flcontext_create();
flcontext_t flcontext_delete(flcontext_t);

flcontext_object_t flcontext_lock_object(flcontext_t, str_t); /* lock and return memory object */
flcontext_object_t flcontext_unlock_object(flcontext_object_t); /* unlock object and return NULL */

void flcontext_set_object(flcontext_object_t, void *, void (*destruction_callback)(void *, void *), void *destruction_callback_data);
void flcontext_get_object(flcontext_object_t);

void flcontext_retain_object(flcontext_object_t); /* increment reference counter */
void flcontext_release_object(flcontext_object_t); /* decrement reference counter */

cl_mem flcontext_load_image(flcontext_t, str_t);
cl_mem flcontext_unload_image(flcontext_t, cl_mem);

#endif /* MAY_MCL_FILTER_CONTEXT_H */

