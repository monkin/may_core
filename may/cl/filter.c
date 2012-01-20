
#include "filter.h"

filter_t filter_create(filter_controller_t controller, cl_context context, json_value_t config, map_t filters) {
	heap_t h = heap_create(16*1024);
	filter_t filter = heap_alloc(h, sizeof(filter_s));
	filter->heap = h;
	filter->controller = controller;
	filter->controller_data = 0;
	filter->context = context;
	filter->config = config;
	filter->filters = filters;
	filter_t background = map_get_cs(filters, "_");
	if(background)
		filter->type = background->type;
	err_try {
		controller->init(filter);
	} err_catch {
		h = heap_delete(h);
		err_throw_down();
	}
	return filter;
}
filter_t filter_delete(filter_t filter) {
	if(filter) {
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
