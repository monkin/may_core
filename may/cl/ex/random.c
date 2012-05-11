
#ifndef MAY_MCLEX_C_INCLUDE
#	error This file should be included from /may/cl/ex.c
#endif

static void random_push_arguments(void *data, void (*push_fn)(void *, mcl_arg_t), void *dt) {}
static void random_global_source(void *data, map_t m, ios_t s) {
	if(mcl_insert_ptr(m, random_global_source)) {
		ios_write_cs(s, "private uint may_random_seed = 0;\n"
			"private bool initialized = false;\n"
			"float may_random() {\n"
				"if(!initialized) {\n"
					"may_random_seed = (uint)get_global_id();\n"
					"initialized = true;\n"
				"}\n"
				"return may_random_seed = ((float)((uint)(may_random_seed*69069 + 5))) / 4294967295.0;\n"
			"}\n"
		);
	}
}
static void random_local_source(void *data, map_t m, ios_t s) {}
static void random_value_source(void *data, ios_t s) {
	ios_write_cs(s, "may_random()");
}

static mcl_ex_vtable_s random_vtable = {
	random_push_arguments,
	random_global_source,
	random_local_source,
	random_value_source
};

mcl_ex_t mcl_random(heap_t h) {
	mcl_ex_t r = heap_alloc(h, sizeof(mcl_ex_s));
	r->return_type = MCLT_FLOAT;
	r->data = 0;
	r->vtable = &random_vtable;
	return r;
}