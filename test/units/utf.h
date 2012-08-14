
void test_utf() {
	TEST_MODULE("utf");
	TEST_CHECK("length") {
		heap_t h = heap_create(0);
		err_try {
			if(utf_length(str_from_cs(h, "UTF-8 спасёт мир!"), UTF_8)!=17)
				TEST_FAIL;
			h = heap_delete(h);
		} err_catch {
			h = heap_delete(h);
			err_throw_down();
		}
	} TEST_END;
	TEST_CHECK("convert") {
		heap_t h = heap_create(0);
		err_try {
			if(str_length(utf_convert(h, str_from_cs(h, "UTF-8 спасёт мир!"), UTF_8, UTF_16_LE))!=34)
				TEST_FAIL;
			h = heap_delete(h);
		} err_catch {
			h = heap_delete(h);
			err_throw_down();
		}
	} TEST_END;
}
