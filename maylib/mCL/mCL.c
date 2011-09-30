
#include "mCL.h"

ERR_DEFINE(e_mcl_error, "mCL error", 0);
ERR_DEFINE(e_mclt_error, "Invalid mCL type operation", e_mcl_error);

bool mclt_is_compatible(mclt_t t1, mclt_t t2) {
	if(t1==t2)
		return true;
	if(mclt_is_pointer(t1) || mclt_is_pointer(t2))
		return false;
	else if(mclt_is_vector(t1)) {
		if(mclt_is_vector(t2))
			return ((t1 & MCLT_V_SIZE) == (t2 & MCLT_V_SIZE)) ? mclt_is_compatible(t1 & 0xFF, t2 & 0xFF) : false;
		else if(mclt_is_numeric(t2))
			return mclt_is_compatible(t1 & 0xFF, t2);
		else
			return false;
	} else if(mclt_is_numeric(t1)) {
		if(mclt_is_numeric(t2)) {
			if(t1==MCLT_FLOAT)
				return (t2 & MCLT_I_SIZE)<=1;
			else if(t2==MCLT_FLOAT)
				return false;
			else
				return (t2 & MCLT_I_SIZE) <= (t2 & MCLT_I_SIZE);
		} else
			return false;
	} else
		return false;
}

bool mclt_is_convertable(mclt_t t1, mclt_t t2) {
	if(t1==t2)
		return true;
}

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
	return t & 0xFFFF;
}

static map_t type_names = 0;
static heap_t type_heap = 0;
static void type_clear() {
	type_heap heap_delete(type_heap);
}

#define TYPE_APPEND(t, nm) { mclt_t tp = (t); map_set(type_names, str_from_bin(type_heap, &tp, sizeof(tp)), str_from_cs(type_heap, nm)); }

str_t mclt_name(mclt_t t) {
	if(!type_heap) {
		err_try {
			type_heap = heap_create(0);
			type_names = map_create(type_heap);
			TYPE_APPEND(MCLT_FLOAT, "float");
			TYPE_APPEND(MCLT_INT, "char");
			TYPE_APPEND(MCLT_INT | MCLT_UNSIGNED, "uchar");
			TYPE_APPEND(MCLT_INT | 1, "short");
			TYPE_APPEND(MCLT_INT | MCLT_UNSIGNED | 1, "ushort");
			TYPE_APPEND(MCLT_INT | 2, "int");
			TYPE_APPEND(MCLT_INT | MCLT_UNSIGNED | 2, "uint");
			TYPE_APPEND(MCLT_INT | 3, "long");
			TYPE_APPEND(MCLT_INT | MCLT_UNSIGNED | 3, "ulong");
		} err_catch {
			type_heap = heap_delete(type_heap);
		}
		atexit(type_clear);
	}
}


