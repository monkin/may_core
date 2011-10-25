
#include "mcl.h"
#include <pthread.h>
#include <CL/cl.h>

ERR_DEFINE(e_mcl_error, "mCL error", 0);
ERR_DEFINE(e_mclt_error, "Invalid mCL type operation", e_mcl_error);

/*
 Returns true if this code is valid:
 	t1 a
 	t2 b
 	a = b
*/
bool mclt_is_compatible(mclt_t t1, mclt_t t2) {
	if(t1==t2)
		return true;
	if(mclt_is_pointer(t1) || mclt_is_pointer(t2))
		return mclt_is_bool(t1) || mclt_is_bool(t2);
	else if(mclt_is_vector(t1)) {
		if(mclt_is_vector(t2))
			return ((t1 & MCLT_V_SIZE) == (t2 & MCLT_V_SIZE)) ? mclt_is_compatible(t1 & 0xFF, t2 & 0xFF) : false;
		else if(mclt_is_numeric(t2))
			return mclt_is_compatible(t1 & 0xFF, t2);
		else
			return false;
	} else if(mclt_is_numeric(t1)) {
		if(mclt_is_numeric(t2)) {
			if(mclt_is_bool(t1))
				return true;
			else if(mclt_is_float(t1))
				return (t2 & MCLT_I_SIZE)<=1;
			else if(mclt_is_float(t2))
				return false;
			else
				return (t2 & MCLT_I_SIZE) <= (t2 & MCLT_I_SIZE);
		} else
			return false;
	} else
		return false;
}

/*
 Returns true if this code is valid:
 	t1 a
 	t2 b
 	a = (t1) b
*/
bool mclt_is_convertable(mclt_t t1, mclt_t t2) {
	if(t1==t2)
		return true;
	if(mclt_is_image(t1) || mclt_is_image(t2))
		return false;
	if(mclt_is_pointer(t1))
		return mclt_is_pointer(t2) ? mclt_pointer_type(t1)==mclt_pointer_type(t2) : false;
	if(mclt_is_pointer(t2))
		return mclt_is_bool(t1);
	if(mclt_is_vector(t1))
		return mclt_is_vector(t2) && mclt_vector_size(t2)==mclt_vector_size(t1);
	if(mclt_is_vector(t2))
		return false;
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
	if(mclt_is_pointer(t) || mclt_is_image(t))
		err_throw(e_mclt_error);
	return t & 0xFFFF & (mclt_t)(~MCLT_POINTER);
}

static map_t type_names = 0;
static heap_t type_heap = 0;
static pthread_mutex_t mclt_nm_mutex;
static pthread_mutexattr_t mclt_nm_mutex_attr;
static void type_clear() {
	type_heap = heap_delete(type_heap);
	pthread_mutexattr_destroy(&mclt_nm_mutex_attr);
	pthread_mutex_destroy(&mclt_nm_mutex);
}

#define TYPE_APPEND(t, nm) { mclt_t tp = (t); map_set_bin(type_names, &tp, sizeof(tp), str_from_cs(type_heap, nm)); }

void mclt_init() {
	if(!type_heap) {
		err_try {
			type_heap = heap_create(0);
			type_names = map_create(type_heap);
			TYPE_APPEND(MCLT_VOID, "void")
			TYPE_APPEND(MCLT_BOOL, "bool")
			TYPE_APPEND(MCLT_FLOAT, "float");
			TYPE_APPEND(MCLT_INTEGER, "char");
			TYPE_APPEND(MCLT_INTEGER | MCLT_UNSIGNED, "uchar");
			TYPE_APPEND(MCLT_INTEGER | 1, "short");
			TYPE_APPEND(MCLT_INTEGER | MCLT_UNSIGNED | 1, "ushort");
			TYPE_APPEND(MCLT_INTEGER | 2, "int");
			TYPE_APPEND(MCLT_INTEGER | MCLT_UNSIGNED | 2, "uint");
			TYPE_APPEND(MCLT_INTEGER | 3, "long");
			TYPE_APPEND(MCLT_INTEGER | MCLT_UNSIGNED | 3, "ulong");
			TYPE_APPEND(MCLT_IMAGE_R, "read_only image_t");
			TYPE_APPEND(MCLT_IMAGE_W, "write_only image_t");
			pthread_mutexattr_init(&mclt_nm_mutex_attr);
			pthread_mutexattr_settype(&mclt_nm_mutex_attr, PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&mclt_nm_mutex, &mclt_nm_mutex_attr);
		} err_catch {
			type_heap = heap_delete(type_heap);
			err_throw_down();
		}
		atexit(type_clear);
	}
}

str_t mclt_name(mclt_t t) {
	str_t r = map_get_bin(type_names, &t, sizeof(t));
	if(r)
		return r;
	pthread_mutex_lock(&mclt_nm_mutex);
	err_try {
		char buff[128];
		char *buff_pos = buff;
		if(mclt_is_pointer(t)) {
			if(t & MCLT_P_GLOBAL) {
				strcpy(buff, "global ");
				buff_pos += 7;
			} else if(t & MCLT_P_LOCAL) {
				strcpy(buff, "local ");
				buff_pos += 6;
			} else if(t & MCLT_P_PRIVATE) {
				strcpy(buff, "private ");
				buff_pos += 8;
			} else
				err_throw(e_mclt_error);
			str_t pt = mclt_name(mclt_pointer_to(t));
			memcpy(buff_pos, str_begin(pt), str_length(pt));
			buff_pos += str_length(pt);
			strcpy(buff_pos, " *");
			TYPE_APPEND(t, buff);
		} else if(mclt_is_vector(t)) {
			str_t vof = mclt_name(mclt_vector_of(t));
			char buff[128];
			assert(str_length(vof)<126);
			memcpy(buff, str_begin(vof), str_length(vof));
			sprintf(buff+str_length(vof), "%ld", (long) mclt_vector_size(t));
			TYPE_APPEND(t, buff);
		} else
			err_throw(e_mclt_error);
		pthread_mutex_unlock(&mclt_nm_mutex);
	} err_catch {
		pthread_mutex_unlock(&mclt_nm_mutex);
		err_throw_down();
	}
	r = map_get_bin(type_names, &t, sizeof(t));
	if(r)
		return r;
	else
		err_throw(e_mclt_error);
}

mclt_t mclt_op_result(mclt_t t1, mclt_t t2) {
	if(t1==t2)
		return t1;
	if(mclt_is_numeric(t1) && mclt_is_numeric(t2)) {
		if(mclt_is_float(t1) || mclt_is_float(t2))
			return MCLT_FLOAT;
		int sz1 = mclt_integer_size(t1);
		int sz2 = mclt_integer_size(t2);
		if(sz1==sz2)
			return mclt_is_unsigned(t1) ? t1 : t2;
		else
			return sz1>sz2 ? t1 : t2;
	}
	if(mclt_is_vector(t1)) {
		if(mclt_is_vector(t2)) {
			if(mclt_vector_size(t1)==mclt_vector_size(t2))
				return mclt_vector(mclt_op_result(mclt_vector_of(t1), mclt_vector_of(t2)),  mclt_vector_size(t1));
			else
				err_throw(e_mclt_error);
		} else
			return mclt_vector(mclt_op_result(mclt_vector_of(t1), t2),  mclt_vector_size(t1));
	}
	if(mclt_is_vector(t2))
		return mclt_op_result(t2, t1);
	if(mclt_is_pointer(t1)) {
		if(mclt_is_integer(t2) && mclt_pointer_to(t1)!=MCLT_VOID)
			return t1;
		else
			err_throw(e_mclt_error);
	}
	if(mclt_is_pointer(t2))
		return mclt_op_result(t2, t1);
	err_throw(e_mclt_error);
}




