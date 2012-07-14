
#include "filter_context.h"

typedef struct {
	cl_mem value;
	mutex_t mutex;
} clmem_object_s;

typedef clmem_object_s *clmem_object_t;

