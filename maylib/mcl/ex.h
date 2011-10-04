
#ifndef MAY_MCLEX_H
#define MAY_MCLEX_H

#include "mcl.h"
#include "../map.h"
#include "../str.h"

struct mcl_ex_ss;
typedef struct mcl_ex_ss *mcl_ex_s;
typedef mcl_ex_s *mcl_ex_t;

struct mcl_ex_ss {

};

typedef void *mcl_arg_t;

mcl_ex_t mcl_call(heap_t h, str_t nm, ...);
mcl_ex_t mcl_call_cs(heap_t h, char *nm, ...);
mcl_ex_t mcl_var(heap_t h, mcl_ex_t);
mcl_ex_t mcl_arg(heap_t h, mclt_t tp, mcl_arg_t *);
mcl_ex_t mcl_const(heap_t h, mclt_t tp, const void *val);


#endif /* MAY_MCLEX_H */

