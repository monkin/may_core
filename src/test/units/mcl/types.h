
#include <may/cl/mcl.h>

void test_mcl_types() {
	TEST_MODULE("mcl_types");
	heap_t h = heap_create(0);
	err_try {
		TEST_CHECK("simple") {
			if(str_compare(mclt_name(MCLT_FLOAT), str_from_cs(h, "float"))!=0)
				TEST_FAIL;
		} TEST_END;
		TEST_CHECK("pointer") {
			if(str_compare(mclt_name(mclt_pointer(MCLT_ULONG, MCL_MEM_GLOBAL)), str_from_cs(h, "global ulong *"))!=0)
				TEST_FAIL;
		} TEST_END;
		TEST_CHECK("vector") {
			if(str_compare(mclt_name(mclt_vector(MCLT_USHORT, 2)), str_from_cs(h, "ushort2"))!=0)
				TEST_FAIL;
		} TEST_END;
		h = heap_delete(h);
	} err_catch {
		h = heap_delete(h);
	}
}
