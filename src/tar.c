
#include "tar.h"
#include <time.h>

ERR_DEFINE(e_tar_error, "Tar archive error", 0);
ERR_DEFINE(e_tar_name_too_long, "File name is too long", e_tar_error);
ERR_DEFINE(e_tar_file_too_large, "File is too large", e_tar_error);
ERR_DEFINE(e_tar_invalid_checksum, "File is corrupted", e_tar_error);

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

static void fill_crc(const tar_header_t t, char *r) {
	unsigned long crc = 0;
	int i;
	for(i=0; i<sizeof(tar_header_s); i++) {
		if(i<148 || i>=156)
			crc += ((const unsigned char *) t)[i];
		else
			crc += (const unsigned char) ' ';
	}
	snprintf(r, 7, "%0*lo", (int) 6, crc);
	r[7] = ' ';
}

static char tar_nulls[512] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#define padd_512(x) (((x) & 0x01FF) ? 512 - ((x) & 0x01FF) : 0)

static void fill_header(tar_header_t th, str_t fname, long long size) {
	if(str_length(fname)>=100)
		err_throw(e_tar_name_too_long);
	if(size>=8LL*1024LL*1024LL)
		err_throw(e_tar_file_too_large);
	memset(th, 0, sizeof(tar_header_s));
	memcpy(th->name, str_begin(fname), str_length(fname));
	memcpy(th->mode, "0000644", 7);
	snprintf(th->size, 12, "%0*lo", (int) 11, (unsigned long) size);
	snprintf(th->mtime, 12, "%0*lo", (int) 11, (unsigned long) time(0));
	th->linkflag[0] = '0';
	fill_crc(th, th->checksum);
}

void tar_put(tar_t tar, str_t fname, str_t content) {
	assert(sizeof(tar_header_s)==512);
	tar_header_s th;
	fill_header(&th, fname, str_length(content));
	long long position = tar->last ? tar->last->position + tar->last->size + padd_512(tar->last->size) : 0;
	ios_seek(tar->stream, position, IOS_SEEK_BEGIN);
	ios_write(tar->stream, &th, sizeof(th));
	ios_write(tar->stream, str_begin(content), str_length(content));
	ios_write(tar->stream, tar_nulls, padd_512(str_length(content)));
	tar_item_t item = heap_alloc(tar->heap, sizeof(tar_item_s));
	item->tar = tar;
	item->name = str_clone(tar->heap, fname);
	item->position = position + sizeof(th);
	item->size = str_length(content);
	item->range = ios_range_create(tar->heap, tar->stream, item->position, item->size);
	item->next = 0;
	map_set(tar->files, item->name, item);
	if(tar->first) {
		tar->last->next = item;
		tar->last = item;
	} else
		tar->first = tar->last = item;
}

void tar_putf(tar_t tar, str_t fname, str_t path) {
	ios_t s = ios_file_create(path, IOS_MODE_R);
	err_try {
		tar_puts(tar, fname, s);
		s = ios_close(s);
	} err_catch {
		s = ios_close(s);
		err_throw_down();
	}
}

void tar_puts(tar_t tar, str_t fname, ios_t stream) {
	assert(sizeof(tar_header_s)==512);
	long long position = tar->last ? tar->last->position + tar->last->size + padd_512(tar->last->size) : 0;
	ios_seek(tar->stream, position, IOS_SEEK_BEGIN);
	ios_write(tar->stream, tar_nulls, 512);
	long long size = 0;
	char *buff = mem_alloc(IOS_DEFAULT_BUFFER_SIZE);
	err_try {
		while(true) {
			size_t n = ios_read_n(stream, buff, 1, IOS_DEFAULT_BUFFER_SIZE);
			ios_write(tar->stream, buff, n);
			size += n;
			if(n!=IOS_DEFAULT_BUFFER_SIZE)
				break;
		}
		buff = mem_free(buff);
	} err_catch {
		buff = mem_free(buff);
		err_throw_down();
	}
	ios_write(tar->stream, tar_nulls, padd_512(size));
	tar_header_s th;
	fill_header(&th, fname, size);
	ios_seek(tar->stream, position, IOS_SEEK_BEGIN);
	ios_write(tar->stream, &th, sizeof(th));
	tar_item_t item = heap_alloc(tar->heap, sizeof(tar_item_s));
	item->tar = tar;
	item->name = str_clone(tar->heap, fname);
	item->position = position + sizeof(th);
	item->size = size;
	item->range = ios_range_create(tar->heap, tar->stream, item->position, size);
	item->next = 0;
	map_set(tar->files, item->name, item);
	if(tar->first) {
		tar->last->next = item;
		tar->last = item;
	} else
		tar->first = tar->last = item;
}

ios_t tar_get_stream(tar_t tar, str_t fname) {
	tar_item_t i = map_get(tar->files, fname);
	return i ? i->range : 0;
}

str_t tar_get(tar_t tar, heap_t h, str_t fname) {
	str_t r = 0;
	tar_item_t i = map_get(tar->files, fname);
	if(!i)
		return 0;
	ios_seek(i->range, 0, IOS_SEEK_END);
	r = str_create(h, ios_tell(i->range));
	ios_seek(i->range, 0, IOS_SEEK_BEGIN);
	ios_read(i->range, str_begin(r), str_length(r));
	return r;
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
		long long pos = 0;
		while(true) {
			ios_seek(s, pos, IOS_SEEK_BEGIN);
			tar_header_s th;
			char checksum[8];
			if(ios_read_n(s, &th, sizeof(th), 1)) {
				fill_crc(&th, checksum);
				if(memcmp(checksum, th.checksum, 8)!=0)
					err_throw(e_tar_invalid_checksum);
				if(th.mode[0]=='0') {
					tar_item_t item = heap_alloc(h, sizeof(tar_item_s));
					item->tar = r;
					item->name = str_from_cs(h, th.name);
					item->position = pos + sizeof(tar_header_s);
					sscanf(th.size, "%llo", &item->size);
					item->range = ios_range_create(h, s, item->position, item->size);
					item->next = 0;
					map_set(r->files, item->name, item);
					if(r->first) {
						r->last->next = item;
						r->last = item;
					} else
						r->first = r->last = item;
					pos += item->size + sizeof(tar_header_s);
					pos += padd_512(pos);
				} else
					pos += sizeof(tar_item_s);
			} else
				break;
		}
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
