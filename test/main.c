
#include "../src/lib.h"

#include "units/heap.h"
#include "units/str.h"
#include "units/map.h"
#include "units/parser.h"
#include "units/stream.h"
#include "units/json.h"
#include "units/tar.h"
#include "units/utf.h"


int main() {
	may_core_init();
	test_str();
	test_map();
	test_parser();
	test_json();
	test_tar();
	test_utf();
	fprintf(stderr, "Results: (%i/%i) %i%%\n", (int) tests_success, (int) (tests_success + tests_failed), (int) (tests_success*100/(tests_success + tests_failed)));
	return 0;
}
