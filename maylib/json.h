
#ifndef MAY_JSON_H
#define MAY_JSON_H

#include "str.h"
#include "parser.h"
#include "stream.h"
#include <stdbool.h>
#include <stdio.h>

struct json_value_ss;

typedef struct json_array_ss {
	size_t size;
	struct json_value_ss *values;
} json_array_s;

typedef json_array_s *json_array_t;

typedef enum {
	JSON_NULL = 0,
	JSON_STRING = 1,
	JSON_NUMBER = 2,
	JSON_ARRAY = 3,
	JSON_OBJECT = 4,
	JSON_FALSE = 5,
	JSON_TRUE = 6
} json_vtype_t;


typedef struct json_value_ss {
	long value_type;
	union {
		str_t string;
		double number;
		map_t object;
		json_array_t array;
	} value;
	struct json_value_ss *parent;
} json_value_s;

typedef struct json_value_s *json_value_t;

parser_t json_parser(heap_t);
json_value_t json_to_value(heap_t, syntree_t);
str_t json_to_string(heap_t, json_value_t);
void json_to_file(FILE *, json_value_t);

enum {
	JSON_FORMATF_FLAG = 0x0100,
	JSON_FORMATF_TABS = 0x0200,
	JSON_FORMATF_TAB_SIZE = 0x00FF
};

#define json_format(format, use_tab, tab_size) ((format) ? JSON_FORMATF_FLAG | ((use_tab) ? JSON_FORMATF_TABS : tab_size) : 0)
#define JSON_FORMAT_NONE 0
#define JSON_FORMAT_TAB json_format(1, 1, 0)
#define JSON_FORMAT_SPACE_2 json_format(1, 0, 2)
#define JSON_FORMAT_SPACE_4 json_format(1, 0, 4)
#define JSON_FORMAT_SPACE_8 json_format(1, 0, 8)

#define json_value_type(jt) (jt)->value_type &

typedef struct jbuilder_ss {
	size_t state;
	heap_t heap;
	FILE *file;
	sbuilder_t sbuilder;
	size_t states_size;
	char *states;
} jbuilder_s;

typedef jbuilder_s *jbuilder_t;

jbuilder_t jbuilder_create(ios_t, int format);
jbuilder_t jbuilder_delete(jbuilder_t);
void jbuilder_array(jbuilder_t);
void jbuilder_array_end(jbuilder_t);
void jbuilder_object(jbuilder_t);
void jbuilder_object_end(jbuilder_t);
void jbuilder_key(jbuilder_t, str_t);
void jbuilder_key_cs(jbuilder_t, const char *);
void jbuilder_number(jbuilder_t, double);
void jbuilder_numberi(jbuilder_t, long long);
void jbuilder_string(jbuilder_t, str_t);
void jbuilder_bool(jbuilder_t, bool);
void jbuilder_null(jbuilder_t);
/*void jbuilder_true(jbuilder_t);
void jbuilder_false(jbuilder_t);*/
#define jbuilder_true(jb) jbuilder_bool((jb), true)
#define jbuilder_false(jb) jbuilder_bool((jb), false)

#endif /* MAY_JSON_H */


