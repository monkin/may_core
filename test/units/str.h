
void test_str() {
	TEST_MODULE("str");
	heap_t h = heap_create(0);
	err_try {
		TEST_CHECK("compare") {
			if(str_compare(str_from_cs(h, "test"), str_from_cs(h, "test"))!=0) {
				TEST_LOG("\"test\"==\"test\" failed");
				TEST_FAIL;
			}
			if(str_compare(str_from_cs(h, "anton"), str_from_cs(h, "natasha"))!=-1) {
				TEST_LOG("\"anton\"<\"natasha\" failed");
				TEST_FAIL;
			}
			if(str_compare_cs(str_from_cs(h, "12345"), "123")!=1) {
				TEST_LOG("\"12345\">\"123\" failed");
				TEST_FAIL;
			}
		} TEST_END;
		h = heap_delete(h);
	} err_catch {
		h = heap_delete(h);
	}
}