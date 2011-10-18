
#ifndef MAY_JSON_H
#define MAY_JSON_H

#include "str.h"
#include "parser.h"
#include "stream.h"
#include <stdbool.h>
#include <stdio.h>

ERR_DECLARE(e_json_error);
ERR_DECLARE(e_json_invalid_state);
ERR_DECLARE(e_json_syntax_error);

struct json_value_ss;
struct json_array_ss;
typedef struct json_array_ss json_array_s;
typedef json_array_s *json_array_t;

typedef enum {
	JSON_NULL = 0,
	JSON_STRING,
	JSON_NUMBER,
	JSON_ARRAY,
	JSON_OBJECT,
	JSON_FALSE,
	JSON_TRUE
} json_vtype_t;

enum json_syntree_names_e {
	JSON_ST_STRING = 1,
	JSON_ST_STRING_SIMPLE = 2,
	JSON_ST_STRING_ESC = 3,
	JSON_ST_NUMBER = 4,
	JSON_ST_TRUE = 5,
	JSON_ST_FALSE = 6,
	JSON_ST_NULL = 7,
	JSON_ST_PAIR = 8,
	JSON_ST_OBJECT = 9,
	JSON_ST_ARRAY = 10
};


typedef struct json_value_ss {
	char value_type;
	union {
		str_t string;
		double number;
		map_t object;
		json_array_t array;
	} value;
	struct json_value_ss *parent;
} json_value_s;

struct json_array_item_ss;
typedef struct json_array_item_ss json_array_item_s;
typedef json_array_item_s *json_array_item_t;

struct json_array_item_ss {
	json_value_s value;
	json_array_item_t next;
};

struct json_array_ss {
	size_t size;
	json_array_item_t first;
	json_array_item_t last;
};

typedef struct json_value_ss *json_value_t;

#define json_format(format, use_tab, tab_size) ((format) ? JSON_FORMATF_FLAG | ((use_tab) ? JSON_FORMATF_TABS : tab_size) : 0)
#define JSON_FORMAT_NONE 0
#define JSON_FORMAT_TAB json_format(1, 1, 0)
#define JSON_FORMAT_SPACE_2 json_format(1, 0, 2)
#define JSON_FORMAT_SPACE_4 json_format(1, 0, 4)
#define JSON_FORMAT_SPACE_8 json_format(1, 0, 8)

void json_init();

parser_t json_parser();
json_value_t json_string2value(heap_t, parser_t, str_t);
json_value_t json_tree2value(heap_t, syntree_t);
str_t json_value2string(heap_t, json_value_t, int format);
void json_value2stream(ios_t, json_value_t, int format);

enum {
	JSON_FORMATF_FLAG = 0x0100,
	JSON_FORMATF_TABS = 0x0200,
	JSON_FORMATF_TAB_SIZE = 0x00FF
};

#define json_value_type(jt) (jt)->value_type

struct jbuilder_vtable_ss;
typedef struct jbuilder_vtable_ss *jbuilder_vtable_t;

typedef struct jbuilder_ss {
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
jbuilder_t jbuilder_create_v(heap_t);
json_value_t jbuilder_value_v(jbuilder_t);

/*jbuilder_t jbuilder_delete(jbuilder_t);*/
#define jbuilder_delete(jb) ((jb)->vtable->x_delete((jb)->data), (jbuilder_t) 0)
/*void jbuilder_array(jbuilder_t);*/
#define jbuilder_array(jb) ((jb)->vtable->array((jb)->data))
/*void jbuilder_array_end(jbuilder_t);*/
#define jbuilder_array_end(jb) ((jb)->vtable->array_end((jb)->data))
/*void jbuilder_object(jbuilder_t);
void jbuilder_object_end(jbuilder_t);*/
#define jbuilder_object(jb) ((jb)->vtable->object((jb)->data))
#define jbuilder_object_end(jb) ((jb)->vtable->object_end((jb)->data))
/*void jbuilder_key(jbuilder_t, str_t);
void jbuilder_key_cs(jbuilder_t, const char *);*/
#define jbuilder_key(jb, s) ((jb)->vtable->key((jb)->data, s))
#define jbuilder_key_cs(jb, s) ((jb)->vtable->key_cs((jb)->data, s))
/*void jbuilder_number(jbuilder_t, double);
void jbuilder_number_i(jbuilder_t, long long);*/
#define jbuilder_number(jb, n) ((jb)->vtable->number((jb)->data, n))
#define jbuilder_number_i(jb, n) ((jb)->vtable->number_i((jb)->data, n))
/*void jbuilder_string(jbuilder_t, str_t);
void jbuilder_string_cs(jbuilder_t, const char *);*/
#define jbuilder_string(jb, s) ((jb)->vtable->string((jb)->data, s))
#define jbuilder_string_cs(jb, s) ((jb)->vtable->string_cs((jb)->data, s))
/*void jbuilder_bool(jbuilder_t, bool);*/
#define jbuilder_bool(jb, b) ((jb)->vtable->x_bool((jb)->data, b))
/*void jbuilder_null(jbuilder_t);*/
#define jbuilder_null(jb) ((jb)->vtable->x_null((jb)->data))
/*void jbuilder_true(jbuilder_t);
void jbuilder_false(jbuilder_t);*/
#define jbuilder_true(jb) jbuilder_bool(jb, true)
#define jbuilder_false(jb) jbuilder_bool(jb, false)

#endif /* MAY_JSON_H */


