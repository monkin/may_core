
#ifndef MAY_PLATFORM_H
#define MAY_PLATFORM_H

/* Platform specific functions */

#include <stdbool.h>

/* bool may_atomic_cmpset(void **place, void *old_value, void *new_value); */
#define may_atomic_cmpset(place, old_val, new_val) __sync_bool_compare_and_swap((void **) place, (void *)(old_val), (void *) (new_val))


#endif /* MAY_PLATFORM_H */

