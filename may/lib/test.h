
#ifndef MAY_TEST_H
#define MAY_TEST_H

/**
 * @example
 * TEST_MODULE("module_name");
 * TEST_CHECK("test_name") {
 *     if(!something) {
 *         TEST_LOG("something is false");
 *         TEST_FAIL;
 *     }
 * } TEST_END;
 */

#include <stdio.h>
#include <stdbool.h>
#include <time.h>

extern const char *test_module_name;
extern const char *test_check_name;
extern clock_t test_start_time;
extern clock_t test_end_time;
extern bool test_check_status;
extern size_t tests_success;
extern size_t tests_failed;

#define TEST_MODULE(name) { test_module_name = name; }
#define TEST_CHECK(name) { err_reset(); test_check_name = name; test_start_time = clock(); fprintf(stderr, "%s.%s STARTED\n", test_module_name, test_check_name); test_check_status = true; err_try
#define TEST_END \
	err_catch {  \
		test_check_status = false; \
		err_reset();         \
	}                        \
	test_end_time = clock(); \
	if(test_check_status) \
		tests_success++;  \
	else                  \
		tests_failed++;   \
	fprintf(stderr, "%s.%s %s %fs\n\n", test_module_name, test_check_name, test_check_status ? "SUCCESS" : "FAIL", (double)(test_end_time-test_start_time)/((double)CLOCKS_PER_SEC)); \
}

#define TEST_LOG(str) fprintf(stderr, "log: %s\n", str)
#define TEST_FAIL test_check_status = false

#endif /* MAY_TEST_H */
