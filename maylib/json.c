
#include "json.h"

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


