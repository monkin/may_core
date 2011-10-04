
#ifndef MAY_MCL_H
#define MAY_MCL_H

#include "../parser.h"
#include <stdbool.h>

/* mclt = mCL type */

ERR_DECLARE(e_mcl_error);
ERR_DECLARE(e_mclt_error);

enum {
	MCLT_VOID = 0,
	MCLT_FLOAT = 4,
	MCLT_INTEGER = 8,
	MCLT_UNSIGNED = 16,
	MCLT_POINTER = 32,
	MCLT_IMAGE_R = 64,
	MCLT_IMAGE_W = 128,
	MCLT_I_SIZE = 3, /* Mask, int size. 0 - 1-byte, 1 - 2, 2 - 4, 3 - 8 */
	MCLT_V_SIZE = 0xFF00, /* Mask, vector size. */
	MCLT_P_GLOBAL = 0x010000, /* Pointer to global memory */
	MCLT_P_LOCAL = 0x020000,  /* Pointer to local memory */
	MCLT_P_PRIVATE = 0x040000 /* Pointer to private memory */
};

enum mclt_type_shortcuts {
	MCLT_CHAR = MCLT_INTEGER,
	MCLT_UCHAR = MCLT_UNSIGNED | MCLT_INTEGER,
	MCLT_SHORT = MCLT_INTEGER | 1,
	MCLT_USHORT = MCLT_UNSIGNED | MCLT_INTEGER | 1,
	MCLT_INT = MCLT_INTEGER | 2,
	MCLT_UINT = MCLT_UNSIGNED | MCLT_INTEGER | 2,
	MCLT_LONG = MCLT_INTEGER | 3,
	MCLT_ULONG = MCLT_UNSIGNED | MCLT_INTEGER | 3
};

enum {
	MCL_MEM_GLOBAL = MCLT_P_GLOBAL,
	MCL_MEM_LOCAL = MCLT_P_LOCAL,
	MCL_MEM_PRIVATE = MCLT_P_PRIVATE
};

typedef long mclt_t;


/*
 Returns true if this code is valid:
 	t1 a
 	t2 b
 	a = b
*/
bool mclt_is_compatible(mclt_t t1, mclt_t t2);
/*
 Returns true if this code is valid:
 	t1 a
 	t2 b
 	a = (t1) b
*/
bool mclt_is_convertable(mclt_t t1, mclt_t t2);

#define mclt_is_integer(t) (((t) & MCLT_INTEGER) && !((t) & MCLT_V_SIZE))
#define mclt_is_float(t) ((t)==MCLT_FLOAT)
#define mclt_is_pointer(t) ((t) & MCLT_POINTER)
#define mclt_is_vector(t) (((t) & MCLT_V_SIZE) && !mclt_is_pointer(t))
#define mclt_is_image(t) ((t & MCLT_IMAGE_R) || (t & MCLT_IMAGE_W))
#define mclt_is_void(t) (!(t))
#define mclt_is_numeric(t) (mclt_is_float(t) || mclt_is_integer(t))
#define mclt_integer_size(t) (1<<(MCLT_I_SIZE & (t)))
mclt_t mclt_vector(mclt_t t1, int vector_size);
mclt_t mclt_pointer(mclt_t t, long mem_type);
mclt_t mclt_vector_of(mclt_t t1);
mclt_t mclt_vector_size(mclt_t t1);
mclt_t mclt_pointer_to(mclt_t t1);
str_t mclt_name(mclt_t);
void mclt_init();

#include "ex.h"


#endif /* MAY_MCL_H */

