
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
		TEST_CHECK("string_builder") {
			sb_t sb1 = sb_create(h);
			sb_append_cs(sb1, "s1");
			
			sb_t sb2 = sb_create(h);
			sb_append_cs(sb2, "sb1");
			sb_append_cs(sb2, "sb2");
			
			sb_preppend_sb(sb1, sb2);
			sb_preppend_cs(sb1, "s2");
			if(str_compare_cs(sb_get(h, sb1), "s2sb1sb2s1"))
				TEST_FAIL;
		} TEST_END;
		h = heap_delete(h);
	} err_catch {
		h = heap_delete(h);
	}
}