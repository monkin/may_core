
#ifndef MAY_MCL_FILTER_H
#define MAY_MCL_FILTER_H

#include "ex.h"
#include "mcl.h"
#include "../lib/json.h"
#include "../lib/map.h"
#include <stdbool.h>
#include <CL/cl.h>

ERR_DECLARE(e_filter_not_found);

struct filter_ss;
typedef struct filter_ss filter_s;
typedef filter_s *filter_t;

struct filter_controller_ss;
typedef struct filter_controller_ss filter_controller_s;
typedef filter_controller_s *filter_controller_t;

struct filter_controller_ss {
	const char *name;
	void (*init)(filter_t);
	void (*destroy)(filter_t);
	mcl_ex_t (*get_expression)(heap_t, filter_t, mcl_ex_t point,
		mcl_arg_t (*create_arg)(heap_t, size_t arg_size, const void *arg_value),
		void (*append_event)(cl_event));
};

struct filter_ss {
	mclt_t type;
	heap_t heap;
	filter_controller_t controller;
	void *controller_data;
	cl_context context;
	json_value_t config;
	map_t filters;
};

filter_t filter_create(str_t, cl_context, json_value_t config, map_t filters);
filter_t filter_create_cs(const char *, cl_context, json_value_t config, map_t filters);
filter_t filter_delete(filter_t);
mcl_ex_t filter_get_expression(heap_t, filter_t, mcl_ex_t point,
	mcl_arg_t (*create_arg)(heap_t, size_t arg_size, const void *arg_value),
	void (*append_event)(cl_event));

void filter_init();
void filter_register(filter_controller_t);

#endif /* MAY_MCL_FILTER_H */

