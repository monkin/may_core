
#include "mem.h"
#include "mutex.h"
#include <pthread.h>

ERR_DEFINE(e_mutex_error, "Mutex init/lock/unlock error", 0);

mutex_t mutex_create(heap_t h, bool is_recursive) {
	pthread_mutexattr_t attr;
	pthread_mutex_t *mutex;
	mutex = heap_alloc(h, sizeof(pthread_mutex_t));
	if(pthread_mutexattr_init(&attr))
		err_throw(e_mutex_error);
	if(pthread_mutexattr_settype(&attr, is_recursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_ERRORCHECK))
		err_throw(e_mutex_error);
	if(pthread_mutex_init(mutex, &attr)) {
		pthread_mutexattr_destroy(&attr);
		err_throw(e_mutex_error);
	}
	pthread_mutexattr_destroy(&attr);
	return (mutex_t) mutex;
}
void mutex_lock(mutex_t m) {
	if(pthread_mutex_lock((pthread_mutex_t *) m))
		err_throw(e_mutex_error);
}
void mutex_unlock(mutex_t m) {
	if(pthread_mutex_unlock((pthread_mutex_t *) m))
		err_throw(e_mutex_error);
}
mutex_t mutex_delete(mutex_t m) {
	if(m)
		pthread_mutex_destroy((pthread_mutex_t *) m);
	return 0;
}

