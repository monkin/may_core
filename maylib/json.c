
#include "json.h"

ERR_DEFINE(e_json_error, "JSON error.", 0);
ERR_DEFINE(e_json_invalid_state, "JSON Builder error.", e_json_error);

/* Parser */

static bool parser_string_simple(syntree_t st, void *d) {
	if(err())
		return false;
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
	if(err())
		return false;
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
				if((i-e)>=6) {
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
	if(err())
		return false;
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

enum json_syntree_names_e {
	JSON_ST_STRING,
	JSON_ST_STRING_SIMPLE,
	JSON_ST_STRING_ESC,
	JSON_ST_NUMBER,
	JSON_ST_TRUE,
	JSON_ST_FALSE,
	JSON_ST_NULL,
	JSON_ST_PAIR,
	JSON_ST_OBJECT,
	JSON_ST_ARRAY
};

parser_t json_parser(heap_t h) {
	parser_t pspaces = parser_rep(h, parser_cset(h, " \t\r\n"), 0, 0);
	parser_t pstring = parser_named(h, JSON_ST_STRING, parser_and(h,
		parser_string(h, "\""), 
		parser_and(h,
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
	parser_t pvalue = parser_or(h,
		parser_or(h,
			parser_or(h, pstring, pnumber),
			parser_or(h, pnull, f_pobject)),
		parser_or(h,
			ptrue,
			parser_or(h, f_parray, pfalse)));
	parser_t parray = parser_named(h, JSON_ST_ARRAY, parser_and(h,
		parser_and(h,
			parser_and(h, parser_string(h, "["), pspaces),
			parser_maybe(h, parser_and(h,
				parser_and(h, pvalue, pspaces),
				parser_rep(h, parser_and(h,
					parser_and(h, parser_string(h, ","), pspaces),
					parser_and(h, pvalue, pspaces)), 0, 0)))),
		parser_string(h, "]")));
	parser_forward_set(f_parray, parray);
	parser_t ppair = parser_named(h, JSON_ST_PAIR, parser_and(h, parser_and(h, pstring, pspaces), parser_and(h, parser_and(h, parser_string(h, ":"), pspaces), pvalue)));
	parser_t pobject = parser_named(h, JSON_ST_OBJECT, parser_and(h,
		parser_and(h,
			parser_and(h, parser_string(h, "{"), pspaces),
			parser_maybe(h, parser_and(h,
				parser_and(h, ppair, pspaces),
				parser_rep(h, parser_and(h,
					parser_and(h, parser_string(h, ","), pspaces),
					parser_and(h, ppair, pspaces)), 0, 0)))),
		parser_string(h, "}")));
	parser_forward_set(f_pobject, pobject);
	return parser_and(h, parser_and(h, pspaces, pvalue), pspaces);
}

/* Builder */

/* typedef struct jbuilder_ss {
	jbuilder_vtable_t vtable;
	void *data;
} jbuilder_s;

typedef jbuilder_s *jbuilder_t;

typedef struct jbuilder_vtable_ss {
	void (*array)(void *);
	void (*array_end)(void *);
	void (*object)(void *);
	void (*object_end)(void *);
	void (*key)(void *, str_t);
	void (*key_cs)(void *, const char *);
	void (*number)(void *, double);
	void (*number_i)(void *, long long);
	void (*string)(void *, str_t);
	void (*string_cs)(void *, const char *);
	void (*x_bool)(void *, bool);
	void (*x_null)(void *);
	void (*x_delete)(void *);
} jbuilder_vtable_s;


jbuilder_t jbuilder_create_s(ios_t, int format);
jbuilder_t jbuilder_create_v(heap_t h); */

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
	bool complete;
	size_t states_capacity;
	size_t states_size;
	char *states;
} jb_s_s;

typedef jb_s_s *jb_s_t;

static void jb_s_push_state(jb_s_t jb, char s) {
	if(jb->states_capacity==jb->states_size) {
		jb->states = mem_realloc(jb->states, sizeof(char[jb->states_capacity ? jb->states_capacity*2 : 32]));
		if(err())
			return;
		jb->states_capacity = jb->states_capacity ? jb->states_capacity*2 : 32;
	}
	jb->states[jb->states_size] = s;
	jb->states_size++;
}

static void jb_s_indent(jb_s_t jb) {
	static char spaces[] = "                                                                ";
	static char tabs[] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	if(jb->format & JSON_FORMATF_FLAG) {
		size_t cnt = (jb->format & JSON_FORMATF_TAB_SIZE) * jb->states_size;
		while(cnt>0) {
			ios_write(jb->stream, (jb->format & JSON_FORMATF_TABS) ? tabs : spaces, cnt>32 ? 32 : cnt, 1);
			cnt-=32;
		}
	}
}

static void jb_s_endl(jb_s_t jb) {
	if(jb->format & JSON_FORMATF_FLAG)
		ios_write(jb->stream, "\n", 1, 1);
}


#define JB_S_TRY_INSERT_VALUE if(jb->states) { \
	switch(jb->states[jb->states_size-1]) {    \
	case JB_S_IN_OBJECT:               \
	case JB_S_IN_OBJECT_FST:           \
		err_set(e_json_invalid_state); \
		return; \
	} \
} 

void jb_s_array(void *jbs) {
	err_reset();
	jb_s_t jb = (jb_s_t) jbs;
	JB_S_TRY_INSERT_VALUE;
	if(jb->states[jb->states_size-1]==JB_S_IN_ARRAY) {
		if(jb->format & JSON_FORMATF_FLAG)
			ios_write(jb->stream, ", ", 2, 1);
		else
			ios_write(jb->stream, ",", 1, 1);
		if(err())
			return;
	}
	ios_write(jb->stream, "{", 1, 1);
	if(!err())
		jb_s_push_state(jb, JB_S_IN_OBJECT_FST);
}
void jb_s_array_end(void *jbs) {
	err_reset();
	jb_s_t jb = (jb_s_t) jbs;
	if(!jb->states)
		err_set(e_json_invalid_state);
	jb->states_size--;
	switch(jb->states[jb->states_size]) {
	case JB_S_IN_ARRAY:
		jb_s_endl(jb);
		if(err())
			return;
		jb_s_indent(jb);
		if(err())
			return;
	case JB_S_IN_ARRAY_FST:
		ios_write(jb->stream, "]", 1, 1);
		break;
	default:
		err_set(e_json_invalid_state);
	}
}
void jb_s_object(void *jbs) {
	err_reset();
	jb_s_t jb = (jb_s_t) jbs;
	JB_S_TRY_INSERT_VALUE;
	if(jb->states[jb->states_size-1]==JB_S_IN_ARRAY) {
		if(jb->format & JSON_FORMATF_FLAG)
			ios_write(jb->stream, ", ", 2, 1);
		else
			ios_write(jb->stream, ",", 1, 1);
		if(err())
			return;
	}
	ios_write(jb->stream, "{", 1, 1);
	if(!err())
		jb_s_push_state(jb, JB_S_IN_OBJECT_FST);
}
void jb_s_object_end(void *jbs) {
	err_reset();
	jb_s_t jb = (jb_s_t) jbs;
	jb->states_size--;
	switch(jb->states[jb->states_size]) {
	case JB_S_IN_OBJECT:
		jb_s_endl(jb);
		if(err())
			return;
		jb_s_indent(jb);
		if(err())
			return;
	case JB_S_IN_OBJECT_FST:
		ios_write(jb->stream, "}", 1, 1);
		break;
	default:
		err_set(e_json_invalid_state);
	}
}
void jb_s_key(void *jbs, str_t v) {
	err_reset();
	jb_s_t jb = (jb_s_t) jbs;
	switch(jb->states[jb->states_size-1]) {
	case JB_S_IN_OBJECT:
	case JB_S_IN_OBJECT_FST:
		break;
	default:
		err_set(e_json_invalid_state);
		return;
	}
}
void jb_s_key_cs(void *jbs, const char *v) {
	err_reset();
	jb_s_t jb = (jb_s_t) jbs;
	switch(jb->states[jb->states_size-1]) {
	case JB_S_IN_OBJECT:
	case JB_S_IN_OBJECT_FST:
		break;
	default:
		err_set(e_json_invalid_state);
		return;
	}
}
void jb_s_number(void *jbs, double v) {
	err_reset();
	jb_s_t jb = (jb_s_t) jbs;
	JB_S_TRY_INSERT_VALUE;
}
void jb_s_number_i(void *jbs, long long v) {
	err_reset();
	jb_s_t jb = (jb_s_t) jbs;
	JB_S_TRY_INSERT_VALUE;
}
void jb_s_string(void *jbs, str_t v) {
	err_reset();
	jb_s_t jb = (jb_s_t) jbs;
	JB_S_TRY_INSERT_VALUE;
}
void jb_s_string_cs(void *jbs, const char *v) {
	err_reset();
	jb_s_t jb = (jb_s_t) jbs;
	JB_S_TRY_INSERT_VALUE;

}
void jb_s_x_bool(void *jbs, bool v) {
	err_reset();
	jb_s_t jb = (jb_s_t) jbs;
	JB_S_TRY_INSERT_VALUE;
}
void jb_s_x_null(void *jbs) {
	err_reset();
	jb_s_t jb = (jb_s_t) jbs;
	JB_S_TRY_INSERT_VALUE;
}
void jb_s_x_delete(void *jbs) {
	mem_free(((jb_s_t) jbs)->states);
	mem_free(jbs);
}



jbuilder_t jbuilder_create_s(ios_t s, int format) {
	jb_s_t r = mem_alloc(sizeof(jb_s_s));
	if(err())
		return 0;
	r->builder.data = r;
	r->stream = s;
	r->format = format;
	r->complete = false;
	r->states_capacity = r->states_size = 0;
	r->states = 0;
	return &r->builder;
}

typedef struct {

} jb_v_s;

jbuilder_t jbuilder_create_v(heap_t h);


