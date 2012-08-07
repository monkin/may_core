
void test_map() {
	TEST_MODULE("map");
	heap_t h = heap_create(0);
	err_try {
		TEST_CHECK("set/get") {
			map_t m = map_create(h);
			map_set(m, str_from_cs(h, "test"), map_set);
			map_set_cs(m, "test2", map_set_cs);
			if(map_get_cs(m, "test")!=map_set) {
				TEST_LOG("map_get_cs(test) invalid");
				TEST_FAIL;
			}
			if(map_get(m, str_from_cs(h, "test2"))!=map_set_cs) {
				TEST_LOG("map_get(test2) invalid");
				TEST_FAIL;
			}
			if(map_get_cs(m, "none")!=0) {
				TEST_LOG("map_get(none) invalid");
				TEST_FAIL;
			}
		} TEST_END;
		h = heap_delete(h);
	} err_catch {
		h = heap_delete(h);
	}
}
