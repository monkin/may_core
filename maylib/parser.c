
#include "parser.h"
#include "heap.h"

bool parser_process(parser_t p, syntree_t r) {
	return p->fn(r, p->data);
}

static bool p_crange(syntree_t st, void *d) {
	str_it_t p = syntree_position(st);
	if(syntree_eof(st))
		return false;
	else if(*p>=((char *)d)[0] && *p<=((char *)d)[1]) {
		syntree_seek(st, p+1);
		return true;
	}
	return false;
}

parser_t parser_crange(heap_t h, char c1, char c2) {
	parser_t r = heap_alloc(h, sizeof(struct parser_s));
	r->fn = p_crange;
	char *cv = r->data = heap_alloc(h, sizeof(char[2]));
	cv[0] = c1;
	cv[1] = c2;
	return r;
}

static bool p_cset(syntree_t st, void *d) {
	str_it_t p = syntree_position(st);
	str_it_t i;
	if(syntree_eof(st))
		return false;
	for(i=str_begin((str_t)d); i!=str_end((str_t)d); i++) {
		if(*i==*p) {
			syntree_seek(st, p+1);
			return true;
		}
	}
	return false;
}

parser_t parser_cset(heap_t h, const char *cs) {
	parser_t r = heap_alloc(h, sizeof(struct parser_s));
	r->fn = p_cset;
	r->data = str_from_cs(h, cs);
	if(!err())
		return r;
	return 0;
}

static bool p_string(syntree_t st, void *d) {
	str_it_t p = syntree_position(st);
	str_it_t i;
	if(syntree_eof(st))
		return false;
	for(i=str_begin((str_t)d); i!=str_end((str_t)d); i++, p++) {
		if(*i!=*p)
			return false;
	}
	syntree_seek(st, p);
	return true;
}

parser_t parser_string(heap_t h, const char *s) {
	parser_t r = heap_alloc(h, sizeof(struct parser_s));
	r->fn = p_string;
	r->data = str_from_cs(h, s);
	return r;
}

parser_t parser_fn(heap_t h, bool (*fn)(syntree_t, void *), void *data) {
	parser_t r = heap_alloc(h, sizeof(struct parser_s));
	r->fn = fn;
	r->data = data;
	return r;
}

static bool p_predicate(syntree_t st, void *d) {
	str_it_t p = syntree_position(st);
	bool (*fn)(char) = d;
	if(syntree_eof(st))
		return false;
	if(fn(*p)) {
		syntree_seek(st, p+1);
		return true;
	}
	return false;
}

parser_t parser_predicate(heap_t h, bool (*p)(char)) {
	parser_t r = heap_alloc(h, sizeof(struct parser_s));
	r->fn = p_predicate;
	r->data = p;
	return r;
}

static bool p_and(syntree_t st, void *d) {
	st = syntree_transaction(st);
	parser_t *data = d;
	if(data[0]->fn(st, data[0]->data)) {
		if(data[1]->fn(st, data[1]->data)) {
			st = syntree_commit(st);
			return true;
		}
	}
	syntree_rollback(st);
	return false;
}

parser_t parser_and(heap_t h, parser_t p1, parser_t p2) {
	parser_t r = heap_alloc(h, sizeof(struct parser_s));
	r->fn = p_and;
	parser_t *dt = r->data = heap_alloc(h, sizeof(parser_t[2]));
	dt[0] = p1;
	dt[1] = p2;
	return r;
}

static bool p_or(syntree_t st, void *d) {
	parser_t *data = d;
	st = syntree_transaction(st);
	if(data[0]->fn(st, data[0]->data)) {
		st = syntree_commit(st);
		return true;
	}
	if(data[1]->fn(st, data[1]->data)) {
		st = syntree_commit(st);
		return true;
	}
	syntree_rollback(st);
	return false;
}

parser_t parser_or(heap_t h, parser_t p1, parser_t p2) {
	parser_t r = heap_alloc(h, sizeof(struct parser_s));
	r->fn = p_or;
	parser_t *dt = r->data = heap_alloc(h, sizeof(parser_t[2]));
	dt[0] = p1;
	dt[1] = p2;
	return r;
}

typedef struct {
	parser_t parser;
	size_t min_count;
	size_t max_count;
} rep_tt;

static bool p_rep(syntree_t st, void *d) {
	rep_tt *data = d;
	size_t i;
	st = syntree_transaction(st);
	for(i=0; (i<data->max_count) || (data->max_count==0); i++) {
		if(!data->parser->fn(st, data->parser->data)) {
			if(i>=data->min_count) {
				st = syntree_commit(st);
				return true;
			} else {
				syntree_rollback(st);
				return false;
			}
		}
	}
	st = syntree_commit(st);
	return true;
}


parser_t parser_rep(heap_t h, parser_t p, size_t min_c, size_t max_c) {
	parser_t r = heap_alloc(h, sizeof(struct parser_s));
	r->fn = p_rep;
	rep_tt *dt = r->data = heap_alloc(h, sizeof(rep_tt));
	dt->parser = p;
	dt->min_count = min_c;
	dt->max_count = max_c;
	return r;
}

parser_t parser_maybe(heap_t h, parser_t p) {
	return parser_rep(h, p, 0, 1);
}

typedef struct {
	int name;
	parser_t parser;
} named_tt;

static bool p_named(syntree_t st, void *d) {
	named_tt *data = d;
	size_t i;
	st = syntree_transaction(st);
	syntree_named_start(st, data->name);
	if(!data->parser->fn(st, data->parser->data)) {
		syntree_rollback(st);
		return false;
	}
	syntree_named_end(st);
	st = syntree_commit(st);
	return true;
}

parser_t parser_named(heap_t h, int nm, parser_t p) {
	parser_t r = heap_alloc(h, sizeof(struct parser_s));
	r->fn = p_named;
	named_tt *dt = r->data = heap_alloc(h, sizeof(rep_tt));
	dt->parser = p;
	dt->name = nm;
	return r;
}

parser_t parser_forward(heap_t h) {
	parser_t r = heap_alloc(h, sizeof(struct parser_s));
	r->data = 0;
	r->fn = 0;
	return r;
}
parser_t parser_forward_set(parser_t p1, parser_t p2) {
	*p1=*p2;
	return p1;
}


