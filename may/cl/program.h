
#ifndef MAY_mcl_progRAM_H
#define MAY_mcl_progRAM_H

#include "mcl.h"
#include "ex.h"
#include <CL/cl.h>

typedef struct {
	cl_program program;
	map_t variables;
} mcl_prog_s;

typedef struct {
	cl_kernel kernel;
	map_t variables;
} mcl_kernel_s;

typedef mcl_prog_s *mcl_prog_t;
typedef mcl_kernel_s *mcl_kernel_t;

mcl_prog_t mcl_prog_create(cl_context, mcl_ex_t);
mcl_prog_t mcl_prog_delete(mcl_prog_t);

void mcl_prog_build(mcl_prog_t, void (*callback)(void *), void *callback_data);

mcl_kernel_t mcl_kernel_create(mcl_prog_t);
mcl_kernel_t mcl_kernel_delete(mcl_kernel_t);
void mcl_kernel_arg(mcl_kernel_t, mcl_arg_t, size_t arg_size, const void *arg_value);

#endif /* MAY_mcl_progRAM_H */

