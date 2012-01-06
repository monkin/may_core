
#include <may/maylib.h>
#include <may/lib/json.h>
#include <may/lib/stream.h>
#include <stdio.h>
#include <stdbool.h>

const char *test_module_name = "none";
const char *test_check_name = "none";
bool test_check_status = true;
size_t tests_success = 0;
size_t tests_failed = 0;

#define TEST_MODULE(name) { test_module_name = name; }
#define TEST_CHECK(name) { err_reset(); test_check_name = name; printf("%s.%s STARTED\n", test_module_name, test_check_name); test_check_status = true; err_try
#define TEST_END \
	err_catch {  \
		test_check_status = false; \
		err_reset();      \
	}                     \
	if(test_check_status) \
		tests_success++;  \
	else                  \
		tests_failed++;   \
	printf("%s.%s %s\n\n", test_module_name, test_check_name, test_check_status ? "SUCCESS" : "FAIL"); \
}

#define TEST_LOG(str) printf("log: %s\n", str)
#define TEST_FAIL test_check_status = false

#include "units/heap.h"
#include "units/str.h"
#include "units/parser.h"
#include "units/stream.h"
#include "units/map.h"
#include "units/json.h"
#include "units/tar.h"
#include "units/mcl/types.h"
#include "units/mcl/program.h"


int main() {
	maylib_init();
	test_parser();
	test_json();
	test_tar();
	test_mcl_types();
	test_mcl_program();
	printf("Results: (%i/%i) %i%%\n", (int) tests_success, (int) (tests_success + tests_failed), (int) (tests_success*100/(tests_success + tests_failed)));
	return 0;
}

