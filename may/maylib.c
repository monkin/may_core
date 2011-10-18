
#include "maylib.h"
#include "lib/json.h"
#include "cl/mcl.h"
#include <stdbool.h>

static bool initialized = false;

void maylib_init() {
	if(!initialized) {
		json_init();
		mclt_init();
		initialized = true;
	}
}



