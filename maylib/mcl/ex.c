
#include "ex.h"

bool mcl_insert_ptr(map_t m, void *p) {
	char phash[sizeof(void *)];
	int i;
	for(i=0; i<sizeof(void *); i++)
		phash[i] = ((char *)(&p))[sizeof(void *) - i];
	if(map_get_bin(m, phash, sizeof(phash)))
		return false;
	else {
		map_set_bin(m, phash, sizeof(phash), p);
		return true;
	}
}

typedef struct {
	char *name;
	size_t args_count;
	mclt_t (*return_type)(size_t, mclt_t *);
} mcl_stdfn_s;

typedef mcl_stdfn_s *mcl_stdfn_t;

static mcl_stdfn_s stdfn_list[] = {
	{"+", 2, 0},
	{"-", 2, 0},
	{"*", 2, 0},
	{"/", 2, 0},
	{"?", 3, 0}
};

static mcl_ex_t mcl_call_internal(heap_t h, mcl_stdfn_t fn, mcl_ex_t *args) {

}

mcl_ex_t mcl_call(heap_t h, str_t nm, ...) {

}

mcl_ex_t mcl_call_cs(heap_t h, const char *nm, ...) {

}
