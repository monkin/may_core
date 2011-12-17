#ifndef MAY_MCLEX_C_INCLUDE
#	error This file should be included from /may/cl/ex.c
#endif

void seq_push_arguments(void *data, void (*push_fn)(void *, mcl_arg_t), void *dt) {

}
void seq_global_source(void *data, map_t m, ios_t s) {

}
void seq_local_source(void *data, map_t m, ios_t s) {

}
void seq_value_source(void *data, ios_t s) {

}

static mcl_ex_vtable_s seq_vtable = {
	seq_push_arguments,
	seq_global_source,
	seq_local_source,
	seq_value_source
};
