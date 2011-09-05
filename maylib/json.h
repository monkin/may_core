
#ifndef MAY_JSON_H
#define MAY_JSON_H

#include "str.h"
#include "parser.h"
#include "stream.h"
#include <stdbool.h>
#include <stdio.h>

ERR_DECLARE(e_json_error);
ERR_DECLARE(e_json_invalid_state);

struct json_value_ss;
struct json_array_ss;
typedef struct json_array_ss json_array_s;
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
	char value_type;
	union {
		str_t string;
		double number;
		map_t object;
		json_array_t array;
	} value;
	struct json_value_ss *parent;
} json_value_s;

struct json_array_ss {
	json_value_s value;
	json_array_t *next;
};

typedef struct json_value_s *json_value_t;

parser_t json_parser(heap_t);
json_value_t json_to_value(heap_t, syntree_t);
str_t json_to_string(heap_t, json_value_t);
void json_to_file(ios_t, json_value_t);

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
jbuilder_t jbuilder_create_v(heap_t h);

/*jbuilder_t jbuilder_delete(jbuilder_t);*/
#define jbuilder_delete(jb) ((jb)->vtable->x_delete((jb)->data), 0)
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
#define jbuilder_true(jb) jbuilder_bool((jb)->data, true)
#define jbuilder_false(jb) jbuilder_bool((jb)->data, false)

#endif /* MAY_JSON_H */


