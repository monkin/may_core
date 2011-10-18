
#include "maylib.h"
#include "json.h"
#include "mcl/mcl.h"
#include <stdbool.h>

static bool initialized = false;

void maylib_init() {
	if(!initialized) {
		json_init();
		mclt_init();
		initialized = true;
	}
}



