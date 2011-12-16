#ifndef MAY_MCLEX_C_INCLUDE
#	error This file should be included from /may/cl/ex.c
#endif

typedef struct {
	str_t content;
	mcl_ex_s self;
} const_data_s;

void const_push_arguments(void *data, void (*push_fn)(void *, mcl_arg_t), void *dt) {}
void const_global_source(void *data, map_t m, ios_t s) {}
void const_local_source(void *data, map_t m, ios_t s) {}
void const_value_source(void *data, ios_t s) {
	const_data_s *cd = data;
	ios_write(s, str_begin(cd->content), str_length(cd->content));
}

static mcl_ex_vtable_s const_vtable = {
	const_push_arguments,
	const_global_source,
	const_local_source,
	const_value_source
};

static cl_ulong read_integer(mclt_t tp, const void *val, void **p) {
	int sz = mclt_integer_size(tp);
	if(p)
		*p = ((char *) val) + sz;
	switch(sz) {
	case 1:
		return *((cl_uchar *) val);
	case 2:
		return *((cl_ushort *) val);
	case 4:
		return *((cl_uint *) val);
	case 8:
		return *((cl_ulong *) val);
	}
}

static char hex_digits[] = "0123456789abcdef";
#define MCL_CONST_WRITE_BYTE(out, b) {      \
	out[0] = hex_digits[(b) & 0x0F];        \
	out[1] = hex_digits[((b) >> 4) & 0x0F]; \
}
#define MCL_CONST_WRITE_HEX(s, p, sz) { \
	int mi_i;                           \
	char mi_buff[16];                   \
	char *mi_p = mi_buff;               \
	assert(sz<=8);                      \
	for(mi_i=0; mi_i<(sz); mi_i++, mi_p+=2) \
		MCL_CONST_WRITE_BYTE(mi_p, ((const char *)p)[mi_i]); \
	ios_write(s, mi_buff, (sz)*2); \
}

static void write_const(ios_t s, mclt_t tp, const void *val) {
	if(mclt_is_bool(tp))
		ios_write(s, (*((const char *) val)) ? "1" : "0", 1);
	else if(mclt_is_integer(tp)) {
		int size = mclt_integer_size(tp);
		ios_write(s, "0x", 2);
		MCL_CONST_WRITE_HEX(s, val, size);
	} if(mclt_is_float(tp)) {
		ios_write_cs(s, "as_float(0x");
		MCL_CONST_WRITE_HEX(s, val, 4);
		ios_write(s, ")", 1);
	} else if(mclt_is_vector(tp)) {
		
	} else
		err_throw(e_mcl_ex_invalid_type);
}

#undef MCL_CONST_BYTE
#undef MCL_CONST_WRITE_HEX

mcl_ex_t mcl_const(heap_t h, mclt_t tp, const void *val) {
	const_data_s *r = heap_alloc(h, sizeof(const_data_s));

	ios_t s = ios_mem_create();
	err_try {
		str_t type_name = mclt_name(tp);
		ios_write(s, "(", 1);
		ios_write(s, str_begin(type_name), str_length(type_name));
		ios_write(s, ")(", 2);
		
		ios_write(s, ")", 1);
		s = ios_close(s);
	} err_catch {
		s = ios_close(s);
		err_throw_down();
	}


	r->self.return_type = tp;
	r->self.data = r;
	r->self.vtable = &const_vtable;
	return &r->self;
}
