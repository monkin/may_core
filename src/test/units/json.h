
#include <maylib/str.h>

void test_json() {
	TEST_MODULE("json");
	TEST_CHECK("to_string") {
		volatile ios_t mems = 0;
		volatile heap_t h = 0;
		err_try {
			h = heap_create(0);
			mems = ios_mem_create();
			jbuilder_t jb = jbuilder_create_s(mems, JSON_FORMAT_NONE);
			jbuilder_object(jb);
				jbuilder_key_cs(jb, "12");
				jbuilder_string_cs(jb, "test\n");
				jbuilder_key_cs(jb, "tri");
				jbuilder_string_cs(jb, "test435");
				jbuilder_key_cs(jb, "qwe");
				jbuilder_number(jb, 10.4);
				jbuilder_key_cs(jb, "\t\r\n");
				jbuilder_number_i(jb, 10492L);
			jbuilder_object_end(jb);
			jbuilder_delete(jb);
			str_t s = ios_mem_to_string(mems, h);
			str_t standard = str_from_cs(h, "{\"12\":\"test\\n\",\"tri\":\"test435\",\"qwe\":10.4,\"\\t\\r\\n\":10492}");
			if(str_compare(s,standard)!=0) {
				TEST_LOG("{\"12\":\"test\\n\",\"tri\":\"test435\",\"qwe\":10.4,\"\\t\\r\\n\":10492}");
				TEST_LOG(str_begin(s));
				TEST_FAIL;
			}
			mems = ios_close(mems);
			h = heap_delete(h);
		} err_catch {
			mems = ios_close(mems);
			h = heap_delete(h);
			err_throw_down();
		}
	} TEST_END;
	TEST_CHECK("simple_parse") {
		volatile heap_t h = 0;
		volatile syntree_t st = 0;
		err_try {
			h = heap_create(0);
			parser_t parser = json_parser(h);
			void *hpos = heap_position(h);
			
			st = syntree_create(str_from_cs(h, "{}"));
			if(parser_process(parser, st)) {
				if(!syntree_eof(st)) {
					TEST_LOG("Empty object parsing error");
					TEST_FAIL;
				} else if(syntree_name(syntree_begin(st))!=JSON_ST_OBJECT) {
					TEST_LOG("Root is not an object");
					TEST_FAIL;
				}
			} else
				TEST_FAIL;
			st = syntree_delete(st);
			heap_release_to(h, hpos);
			
			st = syntree_create(str_from_cs(h, "[]"));
			if(parser_process(parser, st)) {
				if(!syntree_eof(st)) {
					TEST_LOG("Empty array parsing error");
					TEST_FAIL;
				} else if(syntree_name(syntree_begin(st))!=JSON_ST_ARRAY) {
					TEST_LOG("Root is not an array");
					TEST_FAIL;
				}
			} else
				TEST_FAIL;
			st = syntree_delete(st);
			heap_release_to(h, hpos);
			
			st = syntree_create(str_from_cs(h, "12"));
			if(parser_process(parser, st)) {
				if(!syntree_eof(st)) {
					TEST_LOG("Number parsing error");
					TEST_FAIL;
				} else if(syntree_name(syntree_begin(st))!=JSON_ST_NUMBER) {
					TEST_LOG("Root is not an number");
					TEST_FAIL;
				}
			} else
				TEST_FAIL;
			st = syntree_delete(st);
			heap_release_to(h, hpos);
			
			st = syntree_create(str_from_cs(h, "\"\\n12test\\u0030\""));
			if(parser_process(parser, st)) {
				if(!syntree_eof(st)) {
					TEST_LOG("String parsing error");
					TEST_FAIL;
				} else if(syntree_name(syntree_begin(st))!=JSON_ST_STRING) {
					TEST_LOG("Root is not an string");
					TEST_FAIL;
				}
			} else
				TEST_FAIL;
			st = syntree_delete(st);
			heap_release_to(h, hpos);
			
			h = heap_delete(h);
			st = syntree_delete(st);
		} err_catch {
			h = heap_delete(h);
			st = syntree_delete(st);
			err_throw_down();
		}
	} TEST_END;
	TEST_CHECK("parse/to_string") {
		volatile heap_t h = 0;
		volatile syntree_t st = 0;
		err_try {
			h = heap_create(0);
			st = syntree_create(str_from_cs(h, "{\"name\": \"test\", \"items\" : [12, \"\\n\\u0030\"]}"));
			parser_t parser = json_parser(h);
			if(parser_process(parser, st)) {
				if(!syntree_eof(st)) {
					TEST_LOG("Syntax error");
					TEST_FAIL;
				} else if(syntree_name(syntree_begin(st))!=JSON_ST_OBJECT) {
					TEST_LOG("Root is not an object");
					TEST_FAIL;
				} else {
					json_value_t val = json_tree2value(h, st);
					str_t s = json_value2string(h, val, JSON_FORMAT_NONE);
					str_t standard = str_from_cs(h, "{\"name\":\"test\",\"items\":[12,\"\\n0\"]}");
					if(str_compare(s,standard)!=0) {
						TEST_LOG("json_value2string builds some invalid string.");
						TEST_FAIL;
					}
				}
			} else
				TEST_FAIL;
			h = heap_delete(h);
			st = syntree_delete(st);
		} err_catch {
			h = heap_delete(h);
			st = syntree_delete(st);
			err_throw_down();
		}
	} TEST_END;
}
