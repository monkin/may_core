#ifndef MAY_EVENT_H
#define MAY_EVENT_H

#include "heap.h"

struct event_listener_ss;
typedef struct event_listener_ss event_listener_s;
typedef event_listener_s *event_listener_t;

struct event_listener_ss {
	void (*callback)(void *, void *);
	void *data;
	event_listener_t next;
};

typedef struct {
	heap_t heap;
	event_listener_t first;
	event_listener_t last;
	event_listener_t pool;
} event_s;

typedef event_s *event_t;

event_t event_create(heap_t h);
event_listener_t event_append(event_t, void (*callback)(void *, void *), void *);
event_listener_t event_remove(event_t, event_listener_t);
void event_fire(event_t, void *);

#endif /* MAY_EVENT_H */

