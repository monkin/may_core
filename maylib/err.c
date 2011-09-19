
#include "err.h"
#include <stdlib.h>
#include <assert.h>

__thread const err_t *err_ = 0;
__thread int err_line_ = 0;
__thread const char *err_file_ = 0;

const err_t err_0_realisation = { "e_error", "Error", __FILE__, __LINE__, 0 };

int err_is(const err_t *x) {
	const err_t *p = err_;
	if(err_ && !x)
		return 1;
	while(p) {
		if(p==x)
			return 1;
		p = p->parent;
	}
	return 0;
}

ERR_DEFINE(e_arguments, "Invalid arguments.", 0);

__thread size_t err_stack_size = 0;
__thread size_t err_stack_capacity = 0;
__thread jmp_buf *err_stack = 0;

int err_stack_resize() {
	size_t new_size = err_stack_capacity ? err_stack_capacity*2 : 128;
	err_stack = realloc(err_stack, sizeof(jmp_buf[new_size]));
	assert(err_stack);
	return 1;
}

int err_stack_clear() {
	err_stack_size = err_stack_capacity = 0;
	free(err_stack);
	err_stack = 0;
	return 1;
}

void err_throw_down() {
	if(err_stack_size)
		longjmp(err_stack[err_stack_size-1], 1);
	else {
		err_reset();
		exit(1);
	}
}

