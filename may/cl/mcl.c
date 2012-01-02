
#include "mcl.h"
#include <pthread.h>
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
			TYPE_APPEND(MCLT_CHAR, "char");
			TYPE_APPEND(MCLT_UCHAR, "uchar");
			TYPE_APPEND(MCLT_SHORT, "short");
			TYPE_APPEND(MCLT_USHORT, "ushort");
			TYPE_APPEND(MCLT_INT, "int");
			TYPE_APPEND(MCLT_UINT, "uint");
			TYPE_APPEND(MCLT_LONG, "long");
			TYPE_APPEND(MCLT_ULONG, "ulong");
			TYPE_APPEND(MCLT_IMAGE_R, "read_only image2d_t");
			TYPE_APPEND(MCLT_IMAGE_W, "write_only image2d_t");
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




