
#include "program.h"
#include "error.h"
#include <stdbool.h>

typedef struct {
	map_t map;
	ios_t stream;
} push_fn_data_s;

typedef push_fn_data_s *push_fn_data_t;

static void mcl_prog_push_fn(void *data, mcl_arg_t argument) {
	push_fn_data_t d = data;
	bool first = true;
	if(!map_get_bin(d->map, &argument, sizeof(argument))) {
		size_t num = map_length(d->map);
		map_set_bin(d->map, &argument, sizeof(argument), (void *) num);
		if(!first) {
			first = false;
			ios_write(d->stream, ", ", 2);
		}
		str_t type_name = mclt_name(argument->type);
		ios_write(d->stream, str_begin(type_name), str_length(type_name));
		ios_write(d->stream, " ", 1);
		ios_write(d->stream, str_begin(argument->name), str_length(argument->name));
	}
}

mcl_prog_t mcl_prog_create(cl_context ctx, mcl_ex_t ex) {
	heap_t h = heap_create(0);
	heap_t temp_heap = 0;
	ios_t source_stream = 0;
	err_try {
		mcl_prog_t prog = heap_alloc(h, sizeof(mcl_prog_s));
		prog->heap = h;
		prog->variables = map_create(h);

		// Generate source
		source_stream = ios_mem_create();
		temp_heap = heap_create(0);
		ex->vtable->global_source(ex->data, map_create(temp_heap), source_stream);
		ios_write_cs(source_stream, "\nkernel void main(");

		push_fn_data_t pd = heap_alloc(temp_heap, sizeof(push_fn_data_s));
		pd->map = prog->variables;
		pd->stream = source_stream;
		ex->vtable->push_arguments(ex->data, mcl_prog_push_fn, pd);

		ios_write(source_stream, ") {\n", 4);
		ex->vtable->local_source(ex->data, map_create(temp_heap), source_stream);
		ex->vtable->value_source(ex->data, source_stream);
		ios_write(source_stream, "}\n", 2);
		str_t source = ios_mem_to_string(source_stream, temp_heap);
		source_stream = ios_close(source_stream);

		// Create OpenCL program
		const char *source_cs = str_begin(source);
		const size_t source_len = str_length(source);
		cl_int err_code = CL_SUCCESS;
		prog->program = clCreateProgramWithSource(ctx, 1, &source_cs, &source_len, &err_code);
		mcl_throw_if_error(err_code);

		temp_heap = heap_delete(temp_heap);
		return prog;
	} err_catch {
		h = heap_delete(h);
		temp_heap = heap_delete(temp_heap);
		ios_close(source_stream);
		err_throw_down();
	}
}

mcl_prog_t mcl_prog_delete(mcl_prog_t p) {
	if(p) {
		clReleaseProgram(p->program);
		heap_delete(p->heap);
	}
	return 0;
}
