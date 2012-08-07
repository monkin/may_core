
#include "lib.h"
#include "mem.h"
#include "json.h"
#include <stdbool.h>

static bool initialized = false;

void may_core_init() {
	if(!initialized) {
		mem_init();
		json_init();
		initialized = true;
	}
}



