
#ifndef MAY_MCLEX_H
#define MAY_MCLEX_H

#include "mcl.h"
#include "../map.h"
#include "../str.h"
#include "../stream.h"

struct mcl_ex_ss;
typedef struct mcl_ex_ss *mcl_ex_s;
typedef mcl_ex_s *mcl_ex_t;

enum {
	MCL_EXC_CALL,
	MCL_EXC_VAR,
	MCL_EXC_ARG,
	MCL_EXC_CONST
};

typedef void *mcl_arg_t;

bool mcl_insert_ptr(map_t, void *);

typedef struct {
	void (*push_arguments)(void *, void (*push_fn)(void *, mcl_arg_t), void *dt);
	void (*global_source)(void *, map_t, ios_t);
	void (*local_source)(void *, map_t, ios_t);
	void (*value_source)(void *, ios_t);
} mcl_ex_vtable_s;

typedef mcl_ex_vtable_s *mcl_ex_vtable_t;

struct mcl_ex_ss {
	mclt_t return_type;
	mcl_ex_vtable_t vtable;
	void *data;
};

mcl_ex_t mcl_call(heap_t h, str_t nm, ...);
mcl_ex_t mcl_call_cs(heap_t h, const char *nm, ...);
mcl_ex_t mcl_var(heap_t h, mcl_ex_t);
mcl_ex_t mcl_arg(heap_t h, mclt_t tp, mcl_arg_t *);
mcl_ex_t mcl_const(heap_t h, mclt_t tp, const void *val);
mcl_ex_t mcl_seq(heap_t h, size_t n, ...);
mcl_ex_t mcl_if(heap_t h, mcl_ex_t, mcl_ex_t);
mcl_ex_t mcl_unless(heap_t h, mcl_ex_t, mcl_ex_t);
mcl_ex_t mcl_for(heap_t h, mcl_ex_t var_init, mcl_ex_t var_cond, mcl_ex_t var_inc, mcl_ex_t ex);
mcl_ex_t mcl_cast(heap_t h, mclt_t, mcl_ex_t);


#endif /* MAY_MCLEX_H */

