
#ifndef MAY_MCLPROGRAM_H
#define MAY_MCLPROGRAM_H

#include "mcl.h"
#include "ex.h"

typedef struct {
	cl_program program;
	map_t variables;
} mclprog_s;

typedef mclprog_s *mclprog_t;

mclprog_t mclprog_create(cl_context, mcl_ex_t);
void mclprog_build(mclprog_t, void (*callback)(void *), void *callback_data);
mclprog_t mclprog_delete(mclprog_t);


#endif /* MAY_MCLPROGRAM_H */

