
#include <maylib/str.h>

void test_json() {
	TEST_MODULE("json");
	TEST_CHECK("to_string") {
		ios_t mems = 0;
		heap_t h = 0;
		err_try {
			volatile heap_t h = heap_create(0);
			volatile ios_t mems = ios_mem_create();
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
			str_t standard = str_from_cs(h, "{\"12\":\"test\",\"tri\":\"test435\",\"qwe\":10.4,\"\t\r\n\":10492}");
			if(str_compare(s,standard)!=0) 
				TEST_FAIL;
			mems = ios_close(mems);
			h = heap_delete(h);
		} err_catch {
			mems = ios_close(mems);
			h = heap_delete(h);
			err_throw_down();
		}
	} TEST_END;
	TEST_CHECK("parse") {
		volatile heap_t h = 0;
		volatile syntree_t st = 0;
		err_try {
			h = heap_create(0);
			st = syntree_create(str_from_cs(h, "{\"name\": \"test\", \"items\" : [12, \"\\n\"]}"));
			parser_t parser = json_parser(h);
			if(parser_process(parser, st)) {
				if(!syntree_eof(st))
					TEST_FAIL;
				if(syntree_name(syntree_begin(st))!=JSON_OBJECT)
					TEST_FAIL;
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
