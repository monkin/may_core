
#ifndef MAY_LOCK_H
#define MAY_LOCK_H

#include "err.h";
#include "heap.h";
#include <stdbool.h>

ERR_DECLARE(e_mutex_error);

typedef void *mutex_t;

mutex_t mutex_create(heap_t, bool is_recursive);
void mutex_lock(mutex_t);
void mutex_unlock(mutex_t);
mutex_t mutex_delete(mutex_t);

#endif /* MAY_LOCK_H */
