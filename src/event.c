
#include "event.h"
#include <stdbool.h>

event_t event_create(heap_t h) {
	event_t e = heap_alloc(h, sizeof(event_s));
	e->heap = h;
	e->first = e->last = e->pool = 0;
	return e;
}

event_listener_t event_append(event_t e, void (*callback)(void *, void *), void *data) {
	event_listener_t l;
	if(e->pool) {
		l = e->pool;
		e->pool = e->pool->next;
	} else
		l = heap_alloc(e->heap, sizeof(event_listener_s));
	l->callback = callback;
	l->data = data;
	l->next = 0;
	if(e->first)
		e->last->next = l;
	else
		e->first = l;
	e->last = l;
	return l;
}

event_listener_t event_remove(event_t e, event_listener_t l) {
	event_listener_t prev, i;
	for(prev=0, i=e->first; i && i!=l; prev=i, i=i->next);
	if(i) {
		if(prev)
			prev->next = l->next;
		else
			e->first = l;
		if(!l->next)
			e->last = prev;
		l->next = e->pool;
		e->pool = l;
	}
	return 0;
}

void event_fire(event_t e, void *arg) {
	event_listener_t i;
	for(i=e->first; i; i=i->next)
		i->callback(i->data, arg);
}

