
#ifndef MAY_MCL_FILTER_H
#define MAY_MCL_FILTER_H

#include "ex.h"
#include "../lib/json.h"
#include "../lib/map.h"
#include <stdbool.h>
#include <CL/cl.h>

struct filter_ss;
typedef struct filter_ss filter_s;
typedef filter_s *filter_t;

struct filter_controller_ss;
typedef struct filter_controller_ss filter_controller_s;
typedef filter_controller_s *filter_controller_t;

typedef enum {
	FILTER_TYPE_SCALAR = 1,
	FILTER_TYPE_COLOR = 2,
	FILTER_TYPE_ANGLE = 3,
	FILTER_TYPE_NORMAL = 4,
	FILTER_TYPE_POINT = 5
} filter_type_t;

struct filter_controller_ss {
	void (*init)(filter_t);
	void (*destroy)(filter_t);
	mcl_ex_t (*get_expression)(heap_t, filter_t, mcl_ex_t point,
		mcl_arg_t (*create_arg)(heap_t, size_t arg_size, const void *arg_value),
		void (*append_event)(cl_event));
};

struct filter_ss {
	int type;
	heap_t heap;
	filter_controller_t controller;
	void *controller_data;
	cl_context context;
	json_value_t config;
	map_t filters;
};

filter_t filter_create(filter_controller_t, cl_context, json_value_t config, map_t filters);
filter_t filter_delete(filter_t);
mcl_ex_t filter_get_expression(heap_t, filter_t, mcl_ex_t point,
	mcl_arg_t (*create_arg)(heap_t, size_t arg_size, const void *arg_value),
	void (*append_event)(cl_event));

#endif /* MAY_MCL_FILTER_H */
