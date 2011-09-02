
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

enum json_syntree_names_e {
	JSON_ST_STRING,
	JSON_ST_STRING_SIMPLE,
	JSON_ST_STRING_ESC
};

parser_t json_parser(heap_t h) {
	parser_t spaces = parser_rep(h, parser_cset(h, " \t\r\n"), 0, 0);
	parser_t pstring = parser_named(h, JSON_ST_STRING, parser_and(h,
		parser_string(h, "\""), 
		parser_and(h,
			parser_rep(h,
				parser_or(h,
					parser_named(h, JSON_ST_STRING_SIMPLE, parser_fn(h, parser_string_simple, 0)),
					parser_named(h, JSON_ST_STRING_ESC, parser_fn(h, parser_string_esc, 0))
				), 0, 0),
			parser_string(h, "\"")
		)));
}
