
void test_parser() {
	volatile heap_t h = heap_create(0);
	syntree_t st = 0;
	err_try {
		TEST_MODULE("parser");
		TEST_CHECK("rep") {
			st = syntree_create(str_from_cs(h, "  \n \t "));
			parser_process(parser_rep(h, parser_cset(h, " \t\r\n"), 1, 0), st);
			if(!syntree_eof(st))
				TEST_FAIL;
			st = syntree_delete(st);
		} TEST_END;
		
		TEST_CHECK("named/or/and") {
			parser_t p = parser_and(h, parser_string(h, "value_"), parser_rep(h, parser_or(h, parser_named(h, 1, parser_crange(h, 'a', 'z')), parser_named(h, 2, parser_crange(h, '0', '9'))) , 0, 0));
			st = syntree_create(str_from_cs(h, "value_v1"));
			parser_process(p, st);
			if(!syntree_eof(st)) {
				TEST_LOG("Syntax error");
				TEST_FAIL;
			}
			if(syntree_name(syntree_begin(st))!=1 || syntree_name(syntree_next(syntree_begin(st)))!=2) {
				TEST_LOG("Invalid names");
				TEST_FAIL;
			}
			st = syntree_delete(st);
		} TEST_END;
		
		h = heap_delete(h);
	} err_catch {
		h = heap_delete(h);
		st = syntree_delete(st);
		err_throw_down();
	}
}
