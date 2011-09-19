
#include <maylib/json.h>
#include <maylib/stream.h>
#include <stdio.h>
#include <stdbool.h>

const char *test_module_name = "none";
const char *test_check_name = "none";
bool test_check_status = true;
size_t tests_success = 0;
size_t tests_failed = 0;

#define TEST_MODULE(name) { test_module_name = name; }
#define TEST_CHECK(name) { test_check_name = name; printf("%s.%s started\n", test_module_name, test_check_name); test_check_status = true; }
#define TEST_LOG(str) printf("log: %s", str)
#define TEST_ERROR  { printf("Error: %s\nMessage: %s\nFile: %s\nLine: %i\n", err_get()->name, err_get()->message, err_file(), err_line()); err_clear(); }
#define TEST_END { \
	if(err())      \
		TEST_ERROR;\
	if(test_check_status) \
		tests_success++;  \
	else                  \
		tests_failed++;   \
	printf("%s.%s %s\n\n", test_module_name, test_check_name, test_check_status ? "SUCCESS" : "FAIL"); \
	test_check_name = "none"; \
	test_check_status = true; \
}

#include "units/heap.h"
#include "units/str.h"
#include "units/stream.h"
#include "units/map.h"
#include "units/json.h"


int main() {
		
	return 0;
}

