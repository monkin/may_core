
#include "json.h"
#include <string.h>

ERR_DEFINE(e_json_error, "JSON error.", 0);
ERR_DEFINE(e_json_invalid_state, "JSON Builder error.", e_json_error);

/* Value to stream */

str_t json_value2string(heap_t h, json_value_t v, int format) {
	ios_t s = ios_mem_create();
	str_t r = 0;
	err_try {
		json_value2stream(s, v, format);
		r = ios_mem_to_string(s, h);
		s = ios_close(s);
	} err_catch {
		s = ios_close(s);
		err_throw_down();
	}
	return r;
}

static void json_v2s(jbuilder_t jb, json_value_t v) {
	switch(v->value_type) {
	case JSON_NULL:
		jbuilder_null(jb);
		break;
	case JSON_ARRAY: {
		jbuilder_array(jb);
		json_array_item_t i;
		for(i=v->value.array->first; i; i=i->next)
			json_v2s(jb, &(i->value));
		jbuilder_array_end(jb);
		break;
	}
	case JSON_OBJECT: {
		jbuilder_object(jb);
		map_node_t i;
		for(i=map_begin(v->value.object); i; i=map_next(i)) {
			jbuilder_key(jb, i->key);
			json_v2s(jb, (json_value_t) i->value);
		}
		jbuilder_object_end(jb);
		break;
	}
	case JSON_STRING:
		jbuilder_string(jb, v->value.string);
		break;
	case JSON_TRUE:
		jbuilder_bool(jb, true);
		break;
	case JSON_FALSE:
		jbuilder_bool(jb, false);
		break;
	}
}

void json_value2stream(ios_t s, json_value_t v, int format) {
	jbuilder_t jb = jbuilder_create_s(s, format);
	json_v2s(jb, v);
}

/* Tree to value */

static str_t tree2value_string(syntree_node_t i, heap_t h) {
	syntree_node_t j;
	str_t s = str_create(h, str_length(syntree_value(i)));
	str_it_t p = str_begin(s);
	for(j=syntree_child(i); j; j=syntree_next(j)) {
		str_t v = syntree_value(j);
		switch(syntree_name(j)) {
		case JSON_ST_STRING_SIMPLE:
			memcpy(p, str_begin(v), str_length(v));
			p += str_length(v);
			break;
		case JSON_ST_STRING_ESC:
			if(str_length(v)==2) {
				char c;
				switch(str_begin(v)[1]) {
				case '\\': *p='\\'; break;
				case '/': *p='/'; break;
				case '"': *p='\"'; break;
				case 'b': *p='\b'; break;
				case 'f': *p='\f'; break;
				case 'n': *p='\n'; break;
				case 'r': *p='\r'; break;
				case 't': *p='\t'; break;
				default:
					err_throw(e_json_error);
				}
				p++;
			} else {
				assert(str_length(v)==6);
				assert(str_begin(v)[1]=='u');
				str_it_t si;
				long c = 0;
				for(si=str_begin(v)+2; si!=str_end(v); si++) {
					c = c<<4;
					if(*si>='0' && *si<='9')
						c |= (*si) - '0';
					else if(*si>='a' && *si<='f')
						c |= (*si) - 'a';
					else if(*si>='A' && *si<='F')
						c |= (*si) - 'A';
				}
				if(c<=0x7F) {
					*p = c;
					p++;
				} else if(c<=0x7FF) {
					p[0] = 0xC0 | ((c>>6) & 0x1F);
					p[1] = 0x80 | (c & 0x3F);
					p += 2;
				} else {
					p[0] = 0xE0 | ((c>>12) & 0x0F);
					p[1] = 0x80 | ((c>>6) & 0x3F);
					p[2] = 0x80 | (c & 0x3F);
					p += 3;
				}
			}
			break;
		default:
			err_throw(e_json_error);
		}
	}
	*p = '\0';
	s->length = p - str_begin(s);
	return s;
}

static void tree2value_row(syntree_node_t st, jbuilder_t jb, heap_t h) {
	syntree_node_t i;
	for(i=st; i; i=syntree_next(i)) {
		switch(syntree_name(i)) {
		case JSON_ST_STRING:
			jbuilder_string(jb, tree2value_string(i, h));
		case JSON_ST_NUMBER:
			jbuilder_number(jb, str_to_double(syntree_value(i)));
			break;
		case JSON_ST_TRUE:
			jbuilder_true(jb);
			break;
		case JSON_ST_FALSE:
			jbuilder_false(jb);
			break;
		case JSON_ST_NULL:
			jbuilder_null(jb);
			break;
		case JSON_ST_OBJECT: {
			syntree_node_t j;
			jbuilder_object(jb);
			for(j=syntree_child(i); j; j=syntree_next(j)) {
				syntree_node_t key_nd = syntree_child(j);
				jbuilder_key(jb, syntree_value(key_nd));
				tree2value_row(syntree_next(key_nd), jb, h);
			}
			jbuilder_object_end(jb);
			break;
		}
		case JSON_ST_ARRAY:
			jbuilder_array(jb);
			tree2value_row(syntree_child(i), jb, h);
			jbuilder_array_end(jb);
			break;
		default:
			err_throw(e_json_error);
		}
	}
}

json_value_t json_tree2value(heap_t h, syntree_t st) {
	heap_t tmp_h = heap_create(0);
	jbuilder_t jb;
	err_try {
		jb = jbuilder_create_v(h);
		tree2value_row(syntree_begin(st), jb, tmp_h);
		tmp_h = heap_delete(tmp_h);
	} err_catch {
		tmp_h = heap_delete(tmp_h);
	}
	return jbuilder_value_v(jb);
}

/* Parser */

static bool parser_string_simple(syntree_t st, void *d) {
	if(!syntree_eof(st)) {
		switch(syntree_position(st)[0]) {
		case '\\':
		case '"':
			return false;
		default:
			syntree_seek(st, syntree_position(st)+1);
			return true;
		}
	}
	return false;
}

static bool parser_string_esc(syntree_t st, void *d) {
	str_it_t e = str_end(syntree_str(st));
	str_it_t i = syntree_position(st);
	if((e-i)>=2) {
		if(i[0]=='\\') {
			switch(i[1]) {
			case '\\':
			case '/':
			case '"':
			case 'b':
			case 'f':
			case 'n':
			case 'r':
			case 't':
				syntree_seek(st, i+2);
				return true;
			case 'u':
				if((e-i)>=6) {
					str_it_t j;
					for(j=i+2; j<i+6; j++) {
						char c = *j;
						if(!((c>='0' && c<='9') || (c>='a' && c<='f') || (c>='A' && c<='F')))
							return false;
					}
					syntree_seek(st, i+6);
					return true;
				}
			}
		}
	}
	return false;
}

static bool parser_number(syntree_t st, void *d) {
	str_it_t e = str_end(syntree_str(st));
	str_it_t i = syntree_position(st);
	if(i==e)
		return false;
	if(*i=='-')
		i++;
	if(i==e)
		return false;
	if(*i=='0')
		i++;
	else {
		if(i==e)
			return false;
		if(*i>='1' && *i<='9')
			i++;
		else
			return false;
		while(i<e) {
			if(*i>='0' && *i<='9')
				i++;
			else
				break;
		}
	}
	if(i==e) {
		syntree_seek(st, i);
		return true;
	}
	if(*i=='.') {
		if(i==e)
			return false;
		if(*i>='0' && *i<='9')
			i++;
		else
			return false;
		while(i<e) {
			if(*i>='0' && *i<='9')
				i++;
			else
				break;
		}
	}
	if(i==e) {
		syntree_seek(st, i);
		return true;
	}
	if(*i=='e' || *i=='E') {
		if(i==e)
			return false;
		if(*i=='+' || *i=='-')
			i++;
		if(i==e)
			return false;
		if(*i>='0' && *i<='9')
			i++;
		else
			return false;
		while(i<e) {
			if(*i>='0' && *i<='9')
				i++;
			else
				break;
		}
	}
	syntree_seek(st, i);
	return true;
}

parser_t json_parser(heap_t h) {
	parser_t pspaces = parser_rep(h, parser_cset(h, " \t\r\n"), 0, 0);
	parser_t pstring = parser_and(h,
		parser_string(h, "\""), 
		parser_named(h, JSON_ST_STRING, parser_and(h,
			parser_rep(h,
				parser_or(h,
					parser_named(h, JSON_ST_STRING_SIMPLE, parser_fn(h, parser_string_simple, 0)),
					parser_named(h, JSON_ST_STRING_ESC, parser_fn(h, parser_string_esc, 0))
				), 0, 0),
			parser_string(h, "\""))));
	parser_t pnumber = parser_named(h, JSON_ST_NUMBER, parser_fn(h, parser_number, 0));
	parser_t ptrue = parser_named(h, JSON_ST_TRUE, parser_string(h, "true"));
	parser_t pfalse = parser_named(h, JSON_ST_FALSE, parser_string(h, "false"));
	parser_t pnull = parser_named(h, JSON_ST_NULL, parser_string(h, "null"));
	parser_t f_pobject = parser_forward(h);
	parser_t f_parray = parser_forward(h);
	parser_t pvalue = parser_and(h, parser_and(h, pspaces, parser_or(h,
		parser_or(h,
			parser_or(h, pstring, pnumber),
			parser_or(h, pnull, f_pobject)),
		parser_or(h,
			ptrue,
			parser_or(h, f_parray, pfalse)))), pspaces);
	parser_t parray = parser_and(h,
		parser_and(h,
			parser_and(h, parser_string(h, "["), pspaces),
			parser_named(h, JSON_ST_ARRAY, parser_maybe(h, parser_and(h,
				pvalue,
				parser_rep(h, parser_and(h, parser_string(h, ","), pvalue), 0, 0))))),
		parser_string(h, "]"));
	parser_forward_set(f_parray, parray);
	parser_t ppair = parser_named(h, JSON_ST_PAIR, parser_and(h, parser_and(h, pstring, pspaces), parser_and(h, parser_and(h, parser_string(h, ":"), pspaces), pvalue)));
	parser_t pobject = parser_and(h,
		parser_and(h,
			parser_and(h, parser_string(h, "{"), pspaces),
			parser_named(h, JSON_ST_OBJECT, parser_maybe(h, parser_and(h,
				parser_and(h, ppair, pspaces),
				parser_rep(h, parser_and(h,
					parser_and(h, parser_string(h, ","), pspaces),
					parser_and(h, ppair, pspaces)), 0, 0))))),
		parser_string(h, "}"));
	parser_forward_set(f_pobject, pobject);
	return parser_and(h, parser_and(h, pspaces, pvalue), pspaces);
}

/* Builder */

enum {
	JB_S_IN_ARRAY,
	JB_S_IN_ARRAY_FST,
	JB_S_IN_OBJECT,
	JB_S_IN_OBJECT_FST,
	JB_S_IN_OBJECT_VALUE
};

typedef struct {
	jbuilder_s builder;
	ios_t stream;
	int format;
	size_t states_capacity;
	size_t states_size;
	char *states;
} jb_s_s;

typedef jb_s_s *jb_s_t;

static void jb_s_push_state(jb_s_t jb, char s) {
	if(jb->states_capacity==jb->states_size) {
		jb->states = mem_realloc(jb->states, sizeof(char[jb->states_capacity ? jb->states_capacity*2 : 32]));
		jb->states_capacity = jb->states_capacity ? jb->states_capacity*2 : 32;
	}
	jb->states[jb->states_size] = s;
	jb->states_size++;
}

static void jb_s_indent(jb_s_t jb) {
	static char spaces[] = "                                                                ";
	static char tabs[] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	if(jb->format & JSON_FORMATF_FLAG) {
		ptrdiff_t cnt = (jb->format & JSON_FORMATF_TAB_SIZE) * jb->states_size;
		while(cnt>0) {
			ios_write(jb->stream, (jb->format & JSON_FORMATF_TABS) ? tabs : spaces, cnt>32 ? 32 : cnt);
			cnt-=32;
		}
	}
}

static void jb_s_write_string(jb_s_t jb, const char *s, const char *e) {
	ios_write(jb->stream, "\"", 1);
	const char *i = s;
	const char *j = s;
	if(i) {
		while(i<e) {
			bool loop = true;
			while(loop) {
				switch(*j) {
				case '\n':
				case '\r':
				case '\t':
				case '\\':
				case '\"':
					loop = false;
					break;
				default:
					j++;
				}
				if(j==e)
					break;
			}
			if(j!=i)
				ios_write(jb->stream, i, j-i);
			if(i==e)
				return;
			switch(*j) {
			case '\n':
				ios_write(jb->stream, "\\n", 2);
				break;
			case '\r':
				ios_write(jb->stream, "\\r", 2);
				break;
			case '\t':
				ios_write(jb->stream, "\\t", 2);
				break;
			case '\\':
				ios_write(jb->stream, "\\\\", 2);
				break;
			case '\"':
				ios_write(jb->stream, "\\\"", 2);
				break;
			}
			if(*j)
				j++;
			i = j;
			if(i==e)
				break;
		}
	}
	ios_write(jb->stream, "\"", 1);
}

static void jb_s_endl(jb_s_t jb) {
	if(jb->format & JSON_FORMATF_FLAG)
		ios_write(jb->stream, "\n", 1);
}


#define JB_S_TRY_INSERT_VALUE if(jb->states) { \
	switch(jb->states[jb->states_size-1]) {    \
	case JB_S_IN_OBJECT:               \
	case JB_S_IN_OBJECT_FST:           \
		err_throw(e_json_invalid_state); \
		return;                \
	case JB_S_IN_OBJECT_VALUE: \
		jb->states[jb->states_size-1] = JB_S_IN_OBJECT; \
		break;             \
	case JB_S_IN_ARRAY:    \
		ios_write(jb->stream, ",", 1);     \
	case JB_S_IN_ARRAY_FST:                   \
		jb->states[jb->states_size-1] = JB_S_IN_ARRAY; \
		if(jb->format & JSON_FORMATF_FLAG) {           \
			jb_s_endl(jb);    \
			jb_s_indent(jb);  \
		}                     \
	} \
}

void jb_s_array(void *jbs) {
	jb_s_t jb = (jb_s_t) jbs;
	JB_S_TRY_INSERT_VALUE;
	ios_write(jb->stream, "[", 1);
	jb_s_push_state(jb, JB_S_IN_ARRAY_FST);
}
void jb_s_array_end(void *jbs) {
	jb_s_t jb = (jb_s_t) jbs;
	if(!jb->states)
		err_throw(e_json_invalid_state);
	jb->states_size--;
	switch(jb->states[jb->states_size]) {
	case JB_S_IN_ARRAY:
		jb_s_endl(jb);
		jb_s_indent(jb);
	case JB_S_IN_ARRAY_FST:
		ios_write(jb->stream, "]", 1);
		break;
	default:
		err_throw(e_json_invalid_state);
	}
}
void jb_s_object(void *jbs) {
	jb_s_t jb = (jb_s_t) jbs;
	JB_S_TRY_INSERT_VALUE;
	ios_write(jb->stream, "{", 1);
	jb_s_push_state(jb, JB_S_IN_OBJECT_FST);
}
void jb_s_object_end(void *jbs) {
	jb_s_t jb = (jb_s_t) jbs;
	jb->states_size--;
	switch(jb->states[jb->states_size]) {
	case JB_S_IN_OBJECT:
		jb_s_endl(jb);
		jb_s_indent(jb);
	case JB_S_IN_OBJECT_FST:
		ios_write(jb->stream, "}", 1);
		break;
	default:
		err_throw(e_json_invalid_state);
	}
}
void jb_s_key_cs(void *jbs, const char *v) {
	jb_s_t jb = (jb_s_t) jbs;
	switch(jb->states[jb->states_size-1]) {
	case JB_S_IN_OBJECT:
		ios_write(jb->stream, ",", 1);
	case JB_S_IN_OBJECT_FST:
		jb_s_endl(jb);
		jb_s_indent(jb);
		jb_s_write_string(jb, v, v+strlen(v));
		if(jb->format & JSON_FORMATF_FLAG)
			ios_write(jb->stream, ": ", 2);
		else
			ios_write(jb->stream, ":", 1);
		jb->states[jb->states_size-1] = JB_S_IN_OBJECT_VALUE;
		break;
	default:
		err_throw(e_json_invalid_state);
	}
}
void jb_s_key(void *jbs, str_t v) {
	jb_s_t jb = (jb_s_t) jbs;
	switch(jb->states[jb->states_size-1]) {
	case JB_S_IN_OBJECT:
		ios_write(jb->stream, ",", 1);
	case JB_S_IN_OBJECT_FST:
		jb_s_endl(jb);
		jb_s_indent(jb);
		jb_s_write_string(jb, str_begin(v), str_end(v));
		if(jb->format & JSON_FORMATF_FLAG)
			ios_write(jb->stream, ": ", 2);
		else
			ios_write(jb->stream, ":", 1);
		jb->states[jb->states_size-1] = JB_S_IN_OBJECT_VALUE;
		break;
	default:
		err_throw(e_json_invalid_state);
	}
}
void jb_s_number(void *jbs, double v) {
	err_reset();
	jb_s_t jb = (jb_s_t) jbs;
	JB_S_TRY_INSERT_VALUE;
	char buff[128];
	snprintf(buff, 128, "%g", v);
	size_t sl = strlen(buff);
	ios_write(jb->stream, buff, sl);
}
void jb_s_number_i(void *jbs, long long v) {
	err_reset();
	jb_s_t jb = (jb_s_t) jbs;
	JB_S_TRY_INSERT_VALUE;
	char buff[128];
	snprintf(buff, 128, "%lld", v);
	size_t sl = strlen(buff);
	ios_write(jb->stream, buff, sl);
}
void jb_s_string(void *jbs, str_t v) {
	err_reset();
	jb_s_t jb = (jb_s_t) jbs;
	JB_S_TRY_INSERT_VALUE;
	jb_s_write_string(jb, str_begin(v), str_end(v));
}
void jb_s_string_cs(void *jbs, const char *v) {
	err_reset();
	jb_s_t jb = (jb_s_t) jbs;
	JB_S_TRY_INSERT_VALUE;
	jb_s_write_string(jbs, v, v+strlen(v));
}
void jb_s_x_bool(void *jbs, bool v) {
	err_reset();
	jb_s_t jb = (jb_s_t) jbs;
	JB_S_TRY_INSERT_VALUE;
	ios_write(jb->stream, v ? "true" : "false", v ? 4 : 5);
}
void jb_s_x_null(void *jbs) {
	err_reset();
	jb_s_t jb = (jb_s_t) jbs;
	JB_S_TRY_INSERT_VALUE;
	ios_write(jb->stream, "null", 4);
}
void jb_s_x_delete(void *jbs) {
	mem_free(((jb_s_t) jbs)->states);
	mem_free(jbs);
}

static jbuilder_vtable_s jb_s_vtable = {
	jb_s_array, jb_s_array_end, jb_s_object,
	jb_s_object_end, jb_s_key, jb_s_key_cs,
	jb_s_number, jb_s_number_i, jb_s_string,
	jb_s_string_cs, jb_s_x_bool, jb_s_x_null,
	jb_s_x_delete
};

jbuilder_t jbuilder_create_s(ios_t s, int format) {
	jb_s_t r = mem_alloc(sizeof(jb_s_s));
	r->builder.data = r;
	r->builder.vtable = &jb_s_vtable;
	r->stream = s;
	r->format = format;
	r->states_capacity = r->states_size = 0;
	r->states = 0;
	return &r->builder;
}

/* jbuilder_v */

typedef struct {
	jbuilder_s builder;
	heap_t heap;
	json_value_t current;
	str_t key;
	bool complete;
} jb_v_s;

typedef jb_v_s *jb_v_t;

json_value_t jbv_insert_new(jb_v_t jb) {
	json_value_t v = 0;
	if(jb->current) {
		if(jb->current->value_type==JSON_OBJECT) {
			if(!jb->key)
				err_throw(e_json_invalid_state);
			v = heap_alloc(jb->heap, sizeof(json_value_s));
			v->parent = jb->current;
			map_set(jb->current->value.object, jb->key, v);
			jb->key = 0;
		} else if(jb->current->value_type==JSON_ARRAY) {
			json_array_item_t i = heap_alloc(jb->heap, sizeof(json_array_item_s));
			i->value.parent = jb->current;
			i->next = 0;
			json_value_t current = jb->current;
			current->value.array->size++;
			if(current->value.array->first) {
				current->value.array->last->next = i;
				current->value.array->last = i;
			} else
				current->value.array->first = current->value.array->last = i;
			current->value.array->size++;
		} else
			err_throw(e_json_invalid_state);
	} else {
		v = heap_alloc(jb->heap, sizeof(json_value_s));
		v->parent = 0;
		jb->current = v;
	}
	return v;
}

void jb_v_array(void *jbv) {
	jb_v_t jb = (jb_v_t) jbv;
	json_value_t v = jbv_insert_new(jb);
	v->value_type = JSON_ARRAY;
	v->value.array = heap_alloc(jb->heap, sizeof(json_array_s));
	v->value.array->size = 0;
	v->value.array->first = 0;
	v->value.array->last = 0;
	jb->current = v;
}
void jb_v_array_end(void *jbv) {
	jb_v_t jb = (jb_v_t) jbv;
	if(jb->current->parent)
		jb->current = jb->current->parent;
	else
		jb->complete = true;
}
void jb_v_object(void *jbv) {
	jb_v_t jb = (jb_v_t) jbv;
	json_value_t v = jbv_insert_new(jb);
	v->value_type = JSON_OBJECT;
	v->value.object = map_create(jb->heap);
	jb->current = v;
}
void jb_v_object_end(void *jbv) {
	jb_v_t jb = (jb_v_t) jbv;
	if(jb->current->parent)
		jb->current = jb->current->parent;
	else
		jb->complete = true;
}
void jb_v_key(void *jbv, str_t v) {
	jb_v_t jb = (jb_v_t) jbv;
	if(jb->key)
		err_throw(e_json_invalid_state);
	jb->key = str_clone(jb->heap, v);
}
void jb_v_key_cs(void *jbv, const char *v) {
	jb_v_t jb = (jb_v_t) jbv;
	if(jb->key)
		err_throw(e_json_invalid_state);
	jb->key = str_from_cs(jb->heap, v);
}
void jb_v_number(void *jbv, double n) {
	jb_v_t jb = (jb_v_t) jbv;
	json_value_t v = jbv_insert_new(jb);
	v->value_type = JSON_NUMBER;
	v->value.number = n;
}
void jb_v_number_i(void *jbv, long long v) {
	jb_v_number(jbv, (double) v);
}
void jb_v_string(void *jbv, str_t s) {
	jb_v_t jb = (jb_v_t) jbv;
	json_value_t v = jbv_insert_new(jb);
	v->value_type = JSON_STRING;
	v->value.string = str_clone(jb->heap, s);
}
void jb_v_string_cs(void *jbv, const char *s) {
	jb_v_t jb = (jb_v_t) jbv;
	json_value_t v = jbv_insert_new(jb);
	v->value_type = JSON_STRING;
	v->value.string = str_from_cs(jb->heap, s);
}
void jb_v_x_bool(void *jbv, bool b) {
	jb_v_t jb = (jb_v_t) jbv;
	json_value_t v = jbv_insert_new(jb);
	v->value_type = b ? JSON_TRUE : JSON_FALSE;
}
void jb_v_x_null(void *jbv) {
	jb_v_t jb = (jb_v_t) jbv;
	json_value_t v = jbv_insert_new(jb);
	v->value_type = JSON_NULL;
}
void jb_v_x_delete(void *jbv) {}

static jbuilder_vtable_s jb_v_vtable = {
	jb_v_array, jb_v_array_end, jb_v_object,
	jb_v_object_end, jb_v_key, jb_v_key_cs,
	jb_v_number, jb_v_number_i, jb_v_string,
	jb_v_string_cs, jb_v_x_bool, jb_v_x_null,
	jb_v_x_delete
};

jbuilder_t jbuilder_create_v(heap_t h) {
	jb_v_t r = heap_alloc(h, sizeof(jb_v_s));
	r->builder.data = r;
	r->builder.vtable = &jb_v_vtable;
	r->heap = h;
	r->current = 0;
	r->key = 0;
	return &r->builder;
}

json_value_t jbuilder_value_v(jbuilder_t jbv) {
	err_reset();
	jb_v_t jb = (jb_v_t) jbv->data;
	if(!jb->complete)
		err_throw(e_json_invalid_state);
	return jb->current;
}


