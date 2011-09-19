
#include <maylib/str.h>

void test_json() {
	TEST_MODULE("json");
	TEST_CHECK("json_string_builder") {
		ios_t mems = 0;
		heap_t h = 0;
		printf("stack1: %i\n", (int) err_stack_size);
		err_try {
			printf("stack2: %i\n", (int) err_stack_size);
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
		printf("stack3: %i\n", (int) err_stack_size);
	} TEST_END;
}
