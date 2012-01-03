
#ifndef MAY_MCL_PROGRAM_H
#define MAY_MCL_PROGRAM_H

#include "mcl.h"
#include "ex.h"
#include <CL/cl.h>

cl_program mcl_program_create(cl_context, mcl_ex_t);
cl_program mcl_program_delete(cl_program);

cl_kernel mcl_kernel_create(cl_program);
cl_kernel mcl_kernel_delete(cl_kernel);
void mcl_kernel_setarg(cl_kernel, mcl_arg_t, size_t arg_size, const void *arg_value);

#endif /* MAY_MCL_PROGRAM_H */

