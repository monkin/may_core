
void test_tar() {
	TEST_MODULE("tar");
	TEST_CHECK("tar") {
		heap_t h = 0;
		ios_t s = 0;
		tar_t t = 0;
		err_try {
			h = heap_create(0);
			s = ios_mem_create();
			t = tar_create(s);
			tar_put(t, str_from_cs(h, "array.json"), str_from_cs(h, "[]"));
			tar_put(t, str_from_cs(h, "object.json"), str_from_cs(h, "{}"));
			ios_seek(s, 0, IOS_SEEK_END);
			if(ios_tell(s)!=2*1024) {
				TEST_LOG("TAR size is invalid");
				TEST_FAIL;
			} else {
				t = tar_delete(t);
				t = tar_create(s);
				if(str_compare(tar_get(t, h, str_from_cs(h, "array.json")), str_from_cs(h, "[]"))!=0) {
					TEST_LOG("TAR reading error");
					TEST_FAIL;
				}
			}
			t = tar_delete(t);
			h = heap_delete(h);
			s = ios_close(s);
		} err_catch {
			t = tar_delete(t);
			h = heap_delete(h);
			s = ios_close(s);
			err_throw_down();
		}
	} TEST_END;
}
