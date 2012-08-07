
#include "../src/lib.h"
#include "../src/json.h"
#include "../src/stream.h"
#include "../src/test.h"
#include "../src/tar.h"
#include "../src/stream.h"
#include "../src/parser.h"

#include "units/heap.h"
#include "units/str.h"
#include "units/map.h"
#include "units/parser.h"
#include "units/stream.h"
#include "units/json.h"
#include "units/tar.h"


int main() {
	may_core_init();
	test_str();
	test_map();
	test_parser();
	test_json();
	test_tar();
	fprintf(stderr, "Results: (%i/%i) %i%%\n", (int) tests_success, (int) (tests_success + tests_failed), (int) (tests_success*100/(tests_success + tests_failed)));
	return 0;
}