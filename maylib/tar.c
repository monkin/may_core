
#include "tar.h"

typedef struct {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char checksum[8];
	char linkflag[1];
	char linkname[100];
	char pad[255];
} tar_header_s;

typedef tar_header_s *tar_header_t;

static int tar_fill_crc(tar_header_t *t) {
	memset(t->checksum, ' ', 8);
	unsigned long crc = 0;
	int i;
	for(i=0; i<sizeof(tar_header_s); i++)
		crc += ((unsigned char *) t)[i];
	snprintf(oct, 8, "%0*lo", (int) 6, crc);
}

tar_t tar_create(ios_t s) {
	heap_t h = 0;
	tar_t r = 0;
	err_try {
		h = heap_create(0);
		r = heap_alloc(h, sizeof(tar_s));
		r->heap = h;
		r->stream = s;
		r->files = map_create(h);
		r->first = r->last = 0;
		h = heap_delete(h);
	} err_catch {
		h = heap_delete(h);
		err_throw_down();
	}
	return r;
}

tar_t tar_delete(tar_t t) {
	if(t)
		heap_delete(t->heap);
	return 0;
}
