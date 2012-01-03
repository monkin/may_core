
#include "mcl.h"
#include <CL/cl.h>

ERR_DEFINE(e_mcl_error, "mCL error", 0);
ERR_DEFINE(e_mclt_error, "Invalid mCL type operation", e_mcl_error);

mclt_t mclt_vector(mclt_t t, int vector_size) {
	if(!mclt_is_numeric(t) || !(vector_size==2 || vector_size==4 || vector_size==8 || vector_size==16))
		err_throw(e_mclt_error);
	return t | (vector_size<<8);
}

mclt_t mclt_pointer(mclt_t t, long mem_type) {
	if(mclt_is_pointer(t))
		err_throw(e_mclt_error);
	return t | MCLT_POINTER | mem_type;
}

mclt_t mclt_vector_of(mclt_t t) {
	if(!mclt_is_vector(t))
		err_throw(e_mclt_error);
	return t & 0xFF;
}

mclt_t mclt_vector_size(mclt_t t) {
	if(!mclt_is_vector(t))
		err_throw(e_mclt_error);
	return (t >> 8) & 0xFF;
}

mclt_t mclt_pointer_to(mclt_t t) {
	if(!mclt_is_pointer(t))
		err_throw(e_mclt_error);
	return t & 0x3F;
}

static map_t type_names = 0;
static heap_t type_heap = 0;
static void type_clear() {
	type_heap = heap_delete(type_heap);
}

#define TYPE_APPEND(t, nm) { mclt_t tp = (t); map_set_bin(type_names, &tp, sizeof(tp), str_from_cs(type_heap, nm)); }

static void insert_name(mclt_t t, const char *nm) {
	TYPE_APPEND(t, nm);
}
static void insert_name_p(mclt_t t, const char *name) {
	char buff[64];
	memset(buff, 0, 64); sprintf(buff, "global %s *", name);  TYPE_APPEND(mclt_pointer(t, MCLT_P_GLOBAL), buff);
	memset(buff, 0, 64); sprintf(buff, "local %s *", name);   TYPE_APPEND(mclt_pointer(t, MCLT_P_LOCAL), buff);
	memset(buff, 0, 64); sprintf(buff, "private %s *", name); TYPE_APPEND(mclt_pointer(t, MCLT_P_PRIVATE), buff);
}
static void insert_name_v(mclt_t t, const char *name, bool pointable) {
	int i;
	TYPE_APPEND(t, name);
	if(pointable)
		insert_name_p(t, name);
	for(i=1; i<=4; i++) {
		char buff[64];
		long vsz = 1 << i;
		mclt_t vt = mclt_vector(t, vsz);
		memset(buff, 0, 64);
		sprintf(buff, "%s%ld", name, vsz);
		TYPE_APPEND(vt, buff);
		if(pointable)
			insert_name_p(vt, buff);
	}
}

void mclt_init() {
	if(!type_heap) {
		err_try {
			type_heap = heap_create(0);
			type_names = map_create(type_heap);
			insert_name(MCLT_VOID, "void");
			insert_name_p(MCLT_VOID, "void");
			insert_name_v(MCLT_BOOL, "bool", false);
			insert_name_v(MCLT_FLOAT, "float", true);
			insert_name_v(MCLT_CHAR, "char", true);
			insert_name_v(MCLT_UCHAR, "uchar", true);
			insert_name_v(MCLT_SHORT, "short", true);
			insert_name_v(MCLT_USHORT, "ushort", true);
			insert_name_v(MCLT_INT, "int", true);
			insert_name_v(MCLT_UINT, "uint", true);
			insert_name_v(MCLT_LONG, "long", true);
			insert_name_v(MCLT_ULONG, "ulong", true);
			insert_name(MCLT_IMAGE_R, "read_only image2d_t");
			insert_name(MCLT_IMAGE_W, "write_only image2d_t");
		} err_catch {
			type_heap = heap_delete(type_heap);
			err_throw_down();
		}
		map_optimize(type_names);
		atexit(type_clear);
	}
}

str_t mclt_name(mclt_t t) {
	str_t r = map_get_bin(type_names, &t, sizeof(t));
	if(r)
		return r;
	else
		err_throw(e_mclt_error);
}






