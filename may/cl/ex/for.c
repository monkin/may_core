#ifndef MAY_MCLEX_C_INCLUDE
#	error This file should be included from /may/cl/ex.c
#endif

void for_push_arguments(void *data, void (*push_fn)(void *, mcl_arg_t), void *dt) {

}
void for_global_source(void *data, map_t m, ios_t s) {

}
void for_local_source(void *data, map_t m, ios_t s) {

}
void for_value_source(void *data, ios_t s) {

}

static mcl_ex_vtable_s for_vtable = {
	for_push_arguments,
	for_global_source,
	for_local_source,
	for_value_source
};
