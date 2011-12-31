
#include "program.h"

mcl_prog_t mcl_prog_create(cl_context ctx, mcl_ex_t ex) {

}

mcl_prog_t mcl_prog_delete(mcl_prog_t p) {
	if(p) {
		clReleaseProgram(p->program);
		heap_delete(p->heap);
	}
	return 0;
}
