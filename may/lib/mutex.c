
#include "mem.h"
#include "lock.h"
#include <pthread.h>

ERR_DEFINE(e_mutex_error, "Mutex lock/unlock error");

mutex_t mutex_create() {
	pthread_mutex_t *mutex;
	mutex = mem_alloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, 0);
	return (mutex_t) mutex;
}
void mutex_lock(mutex_t l) {
	if(pthread_mutex_lock((pthread_mutex_t *) l))
		err_throw(e_mutex_error);
}
void mutex_unlock(mutex_t l) {
	if(pthread_mutex_unlock((pthread_mutex_t *) l))
		err_throw(e_mutex_error);
}
mutex_t mutex_delete(mutex_t l) {
	if(l) {
		pthread_mutex_destroy((pthread_mutex_t *) l);
		mem_free(l);
	}
	return 0;
}

