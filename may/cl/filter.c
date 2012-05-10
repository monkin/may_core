
#include "filter.h"

ERR_DEFINE(e_filter_not_found, "Filter not found", e_mcl_error);
ERR_DEFINE(e_filter_invalid_arguments, "Invalid filter arguments", e_mcl_error);

static heap_t filter_heap = 0;
static map_t filter_map = 0;

static void uregister_filters() {
	filter_heap = heap_delete(filter_heap);
}
void filter_init() {
	filter_heap = heap_create(0);
	err_try {
		filter_map = map_create(filter_heap);
		atexit(uregister_filters);
	} err_catch {
		filter_heap = heap_delete(filter_heap);
		err_throw_down();
	}
}
void filter_register(filter_controller_t c) {
	map_set_cs(filter_map, c->name, c);
}

static filter_t filter_create_internal(filter_controller_t controller, cl_context context, json_value_t config, map_t filters, map_t binary_streams) {
	heap_t h = heap_create(16*1024);
	filter_t filter = heap_alloc(h, sizeof(filter_s));
	filter->heap = h;
	filter->controller = controller;
	filter->controller_data = 0;
	filter->context = context;
	filter->config = config;
	filter->filters = filters;
	filter->binary_streams = binary_streams;
	filter_t background = map_get_cs(filters, "_");
	filter->type = background ? background->type : 0;
	err_try {
		controller->init(filter);
	} err_catch {
		h = heap_delete(h);
		err_throw_down();
	}
	return filter;
}

filter_t filter_create(str_t s, cl_context context, json_value_t config, map_t filters, map_t binary_streams) {
	filter_controller_t controller = map_get(filter_map, s);
	if(controller)
		return filter_create_internal(controller, context, config, filters, binary_streams);
	else
		err_throw(e_filter_not_found);
}

filter_t filter_create_cs(const char *s, cl_context context, json_value_t config, map_t filters, map_t binary_streams) {
	filter_controller_t controller = map_get_cs(filter_map, s);
	if(controller)
		return filter_create_internal(controller, context, config, filters, binary_streams);
	else
		err_throw(e_filter_not_found);
}

filter_t filter_delete(filter_t filter) {
	if(filter) {
		if(filter->controller->destroy)
			filter->controller->destroy(filter);
		heap_delete(filter->heap);
	}
	return 0;
}
mcl_ex_t filter_get_expression(heap_t heap, filter_t filter, mcl_ex_t point,
		mcl_arg_t (*create_arg)(heap_t, size_t arg_size, const void *arg_value),
		void (*append_event)(cl_event)) {
	return filter->controller->get_expression(heap, filter, point, create_arg, append_event);
}

#include "filters/const.c"


