#ifndef MAY_ERR_H
#define MAY_ERR_H

#define ERR_HAVE_BACKTRACE

#ifdef ERR_HAVE_BACKTRACE
#	include <unistd.h>
#endif

#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <setjmp.h>

/**
 * Example
	
	err_try {
		// do something with err_throw(err_name)
	} err_ctach {
		printf("fail");
		err_reset();
	}
 
*/

#ifdef ERR_HAVE_BACKTRACE
#	include <execinfo.h>
#	define ERR_TRACE_INFO_SIZE 50
	extern size_t err_trace_size;
	extern void *err_trace_info[ERR_TRACE_INFO_SIZE];
#	define ERR_TRACE_STORE { err_trace_size = backtrace(err_trace_info, ERR_TRACE_INFO_SIZE); }
#	define ERR_TRACE_CLEAR { err_trace_size = 0; }
#	define ERR_TRACE_PRINT { backtrace_symbols_fd(err_trace_info, err_trace_size, STDERR_FILENO); }

#else
#	define ERR_TRACE_STORE
#	define ERR_TRACE_CLEAR
#	define ERR_TRACE_PRINT
#endif /* ERR_HAVE_BACKTRACE */



typedef struct err_s {
	const char *name;
	const char *message;
	const char *file;
	int line;
	const struct err_s *parent;
} err_t;

extern __thread const err_t *err_;
extern __thread int err_line_;
extern __thread const char *err_file_;

extern const err_t err_0_realisation;

#define ERR_DECLARE(name) extern const err_t *const name; extern const err_t err_ ## name ## _realisation
#define ERR_DEFINE(name, message, parent_error) \
const err_t err_ ## name ## _realisation = { # name, message, __FILE__, __LINE__, &err_ ## parent_error ## _realisation }; \
const err_t *const name = &err_ ## name ## _realisation

#ifndef NDEBUG
#	define err_display() { fprintf(stderr, "Error: %s\nMessage: %s\nFile: %s\nLine: %i\n", err_->name, err_->message, err_file_, err_line_); ERR_TRACE_PRINT; }
#	define err_message(m) fprintf(stderr, "%s", m)
#else
#	define err_display() ;
#	define err_message(m) ;
#endif

/**
 * Return true if error exist
 */
#define err() (err_!=0)
/**
 * Show error, if it not processed
 */
#define err_reset() { if(err()) { err_display(); err_=0; }; ERR_TRACE_CLEAR; }
/**
 * Replase old error to new
 */
#define err_replace(name) { ERR_TRACE_STORE; err_ = name; err_line_ = __LINE__; err_file_ = __FILE__; }
/**
 * Set error information
 */
#define err_set(name) { err_reset(); err_replace(name) }
/**
 * Get error
 */
#define err_get() err_
#define err_file() err_file_
#define err_line() err_line_
/**
 * Clear error flag
 */
#define err_clear() { ERR_TRACE_CLEAR; err_ = 0; err_file_ = 0; err_line_ = 0; }

int err_is(const err_t *);

ERR_DECLARE(e_arguments);

extern __thread size_t err_stack_size;
extern __thread size_t err_stack_capacity;
extern __thread jmp_buf *err_stack;

int err_stack_resize();
int err_stack_clear();

void err_throw_down();
#define err_try if((err_stack_size==err_stack_capacity ? err_stack_resize() : 0) ? !setjmp(err_stack[err_stack_size++]) : !setjmp(err_stack[err_stack_size++])) {
#define err_catch err_stack_size--; if(!err_stack_size) err_stack_clear(); } else if((--err_stack_size) ? 1 : err_stack_clear())
#define err_throw(err_name) { err_replace(err_name); err_throw_down(); }


#endif /* MAY_ERR_H */

