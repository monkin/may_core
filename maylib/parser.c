
#include "parser.h"
#include "heap.h"

bool parser_process(parser_t p, syntree_t r) {
	return p->fn(r, p->data);
}

static bool p_crange(syntree_t st, void *d) {
	if(!err()) {
		str_it_t p = syntree_position(st);
		if(syntree_eof(st))
			return false;
		else if(*p>=((char *)d)[0] && *p<=((char *)d)[1]) {
			syntree_seek(st, p+1);
			return true;
		}
	}
	return false;
}

parser_t parser_crange(heap_t h, char c1, char c2) {
	if(!err()) {
		parser_t r = heap_alloc(h, sizeof(struct parser_s));
		if(!err()) {
			r->fn = p_crange;
			char *cv = r->data = heap_alloc(h, sizeof(char[2]));
			if(!err()) {
				cv[0] = c1;
				cv[1] = c2;
			} else
				return 0;
		}
		return r;	
	} else
		return 0;
}

static bool p_cset(syntree_t st, void *d) {
	if(!err()) {
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
	}
	return false;
}

parser_t parser_cset(heap_t h, const char *cs) {
	if(!err()) {
		parser_t r = heap_alloc(h, sizeof(struct parser_s));
		if(!err()) {
			r->fn = p_crange;
			r->data = str_from_cs(h, cs);
			if(!err())
				return r;
		}
	}
	return 0;
}

static bool p_string(syntree_t st, void *d) {
	if(!err()) {
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
	return false;
}

parser_t parser_string(heap_t h, const char *s) {
	if(!err()) {
		parser_t r = heap_alloc(h, sizeof(struct parser_s));
		if(!err()) {
			r->fn = p_string;
			r->data = str_from_cs(h, s);
			if(!err())
				return r;
		}
	}
	return 0;
}

parser_t parser_fn(heap_t h, bool (*fn)(syntree_t, void *), void *data) {
	if(!err()) {
		parser_t r = heap_alloc(h, sizeof(struct parser_s));
		if(!err()) {
			r->fn = fn;
			r->data = data;
			return r;
		}
	}
	return 0;
}

static bool p_predicate(syntree_t st, void *d) {
	if(!err()) {
		str_it_t p = syntree_position(st);
		bool (*fn)(char) = d;
		if(syntree_eof(st))
			return false;
		if(fn(*p)) {
			syntree_seek(st, p+1);
			return true;
		}
	}
	return false;
}

parser_t parser_predicate(heap_t h, bool (*p)(char)) {
	if(!err()) {
		parser_t r = heap_alloc(h, sizeof(struct parser_s));
		if(!err()) {
			r->fn = p_predicate;
			r->data = p;
			return r;
		}
	}
	return 0;
}

static bool p_and(syntree_t st, void *d) {
	if(!err()) {
		st = syntree_transaction(st);
		if(err())
			return false;
		parser_t *data = d;
		if(data[0]->fn(st, data[0]->data)) {
			if(err())
				return false;
			if(data[1]->fn(st, data[1]->data)) {
				if(err())
					return false;
				syntree_commit(st);
				return true;
			}
		}
		if(err())
			return false;
		syntree_rollback(st);
	}
	return false;
}

parser_t parser_and(heap_t h, parser_t p1, parser_t p2) {
	if(!err()) {
		parser_t r = heap_alloc(h, sizeof(struct parser_s));
		if(!err()) {
			r->fn = p_and;
			parser_t *dt = r->data = heap_alloc(h, sizeof(parser_t[2]));
			if(!err()) {
				dt[0] = p1;
				dt[1] = p2;
				return r;
			}
		}
	}
	return 0;
}

static bool p_or(syntree_t st, void *d) {
	if(!err()) {
		parser_t *data = d;
		st = syntree_transaction(st);
		if(err())
			return false;
		if(data[0]->fn(st, data[0]->data)) {
			if(err())
				return false;
			syntree_commit(st);
			return !err();
		}
		if(data[1]->fn(st, data[1]->data)) {
			if(err())
				return false;
			syntree_commit(st);
			return !err();
		}
		if(err())
			return false;
		syntree_rollback(st);
	}
	return false;
}

parser_t parser_or(heap_t h, parser_t p1, parser_t p2) {
	if(!err()) {
		parser_t r = heap_alloc(h, sizeof(struct parser_s));
		if(!err()) {
			r->fn = p_or;
			parser_t *dt = r->data = heap_alloc(h, sizeof(parser_t[2]));
			if(!err()) {
				dt[0] = p1;
				dt[1] = p2;
				return r;
			}
		}
	}
	return 0;
}

typedef struct {
	parser_t parser;
	size_t min_count;
	size_t max_count;
} rep_tt;

static bool p_rep(syntree_t st, void *d) {
	if(!err()) {
		rep_tt *data = d;
		size_t i;
		st = syntree_transaction(st);
		if(err())
			return false;
		for(i=0; (i<data->max_count) || (data->max_count==0); i++) {
			if(!data->parser->fn(st, data->parser->data)) {
				if(i>=data->min_count) {
					syntree_commit(st);
					return !err();
				} else {
					syntree_rollback(st);
					return false;
				}
			}
			if(err())
				return false;
		}
		syntree_commit(st);
		return !err();
	}
	return false;
}


parser_t parser_rep(heap_t h, parser_t p, size_t min_c, size_t max_c) {
	if(!err()) {
		parser_t r = heap_alloc(h, sizeof(struct parser_s));
		if(!err()) {
			r->fn = p_rep;
			rep_tt *dt = r->data = heap_alloc(h, sizeof(rep_tt));
			if(!err()) {
				dt->parser = p;
				dt->min_count = min_c;
				dt->max_count = max_c;
				return r;
			}
		}
	}
	return 0;
}

parser_t parser_maybe(heap_t h, parser_t p) {
	return parser_rep(h, p, 0, 1);
}

typedef struct {
	int name;
	parser_t parser;
} named_tt;

static bool p_named(syntree_t st, void *d) {
	if(!err()) {
		named_tt *data = d;
		size_t i;
		st = syntree_transaction(st);
		if(err())
			return false;
		syntree_named_start(st, data->name);
		if(err())
			return false;
		if(!data->parser->fn(st, data->parser->data)) {
			syntree_rollback(st);
			return false;
		}
		syntree_named_end(st);
		if(err())
			return false;
		syntree_commit(st);
		return !err();
	}
	return false;
}

parser_t parser_named(heap_t h, int nm, parser_t p) {
	if(!err()) {
		parser_t r = heap_alloc(h, sizeof(struct parser_s));
		if(!err()) {
			r->fn = p_rep;
			named_tt *dt = r->data = heap_alloc(h, sizeof(rep_tt));
			if(!err()) {
				dt->parser = p;
				dt->name = nm;
				return r;
			}
		}
	}
	return 0;
}

parser_t parser_forward(heap_t h) {
	if(!err()) {
		parser_t r = heap_alloc(h, sizeof(struct parser_s));
		if(!err()) {
			r->data = 0;
			r->fn = 0;
			return r;
		}
	}
	return 0;
}
parser_t parser_forward_set(parser_t p1, parser_t p2) {
	if(!err()) {
		*p1=*p2;
		return p1;
	} else
		return 0;
}


