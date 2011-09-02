
#include "stream.h"
#include "mem.h"

ERR_DEFINE(e_ios_error, "IO error.", 0);
ERR_DEFINE(e_ios_invalid_mode, "Invalid file open mode.", e_ios_error);

// File streams.

typedef struct {
	FILE *file;
	void *block;
} ios_f_s;

typedef ios_f_s *ios_f_t;

static void ios_f_write(void *f, void *dt, size_t sz) {
	if(!fwrite(dt, sz, 1, ((ios_f_t) f)->file))
		err_set(e_ios_error);
}
static void ios_f_read(void *f, void *dt, size_t sz) {
	if(!fread(dt, sz, 1, ((ios_f_t) f)->file))
		err_set(e_ios_error);
}
bool ios_f_eof(void *f) {
	return feof(((ios_f_t) f)->file);
}
long long ios_f_tell(void *f) {
	return ftello64(((ios_f_t) f)->file);
}
void ios_f_seek(void *f, long long pos, int origin) {
	if(0!=fseeko64(((ios_f_t) f)->file, pos, origin))
		err_set(e_ios_error);
}
void ios_f_flush(void *f) {
	flush(((ios_f_t) f)->file);
}
void ios_f_close(void *f) {
	close(((ios_f_t) f)->file);
	mem_free(((ios_f_t) f)->block);
}

static ios_table_s ios_rf_vtable = { 0, ios_f_read, ios_f_eof, ios_f_tell, ios_f_seek, 0, ios_f_close };
static ios_table_s ios_wf_vtable = { ios_f_write, 0, ios_f_eof, ios_f_tell, ios_f_seek, ios_f_flush, ios_f_close };
static ios_table_s ios_rwf_vtable = { ios_f_write, ios_f_read, ios_f_eof, ios_f_tell, ios_f_seek, ios_f_flush, ios_f_close };

ios_t ios_file_create(str_t s, ios_mode_t mode) {
	err_reset();
	const char *md = 0;
	switch(mode) {
	case IOS_MODE_R: md = "r"; break;
	case IOS_MODE_W: md = "w"; break;
	case IOS_MODE_RP: md = "r+"; break;
	case IOS_MODE_WP: md = "w+"; break;
	default:
		err_set(e_ios_invalid_mode);
		return 0;
	}
	FILE *f = fopen(str_begin(s), md);
	if(!f) {
		err_set(e_ios_error);
		return 0;
	}
	char *mem = mem_alloc(sizeof(ios_s) + sizeof(ios_f_s));
	if(err()) {
		fclose(f);
		return 0;
	}
	ios_t r = (ios_t) mem;
	ios_f_t fr = (ios_f_t)(mem + sizeof(ios_s));
	r->data = fr;
	switch(mode) {
	case IOS_MODE_R: r->vtable = &ios_rf_vtable; break;
	case IOS_MODE_W: r->vtable = &ios_wf_vtable; break;
	case IOS_MODE_RP: r->vtable = &ios_rwf_vtable; break;
	case IOS_MODE_WP: r->vtable = &ios_rwf_vtable; break;
	}
	fr->file = f;
	fr->block = mem;
	return r;
}

void ios_std_close(void *f) {
	close(((ios_f_t) f)->file);
}

static ios_table_s std_rs = { 0, ios_f_read, ios_f_eof, 0, 0, ios_f_flush, ios_std_close };
static ios_table_s std_ws = { ios_f_write, 0, ios_f_eof, 0, 0, ios_f_flush, ios_std_close };

static ios_f_s ios_std_infs = { 0, 0 };
static ios_f_s ios_std_outfs = { 0, 0 };
static ios_f_s ios_std_errfs = { 0, 0 };

static ios_s ios_std_ins = { &ios_std_infs, &std_rs };
static ios_s ios_std_outs = { &ios_std_outfs, &std_ws };
static ios_s ios_std_errs = { &ios_std_errfs, &std_ws };

ios_t ios_std_in() {
	ios_std_infs.file = stdin;
	return &ios_std_ins;
}
ios_t ios_std_out() {
	ios_std_outfs.file = stdout;
	return &ios_std_ins;
}
ios_t ios_std_err() {
	ios_std_errfs.file = stderr;
	return &ios_std_ins;
}

// Memory streams

enum {
	IOS_MEM_BLOCK_SIZE = 64*1024
};

typedef struct ios_mem_block_ss {
	struct ios_mem_block_ss *next;
	struct ios_mem_block_ss *prev;
	char data[IOS_MEM_BLOCK_SIZE];
} ios_mem_block_s;

typedef ios_mem_block_s *ios_mem_block_t;

typedef struct {
	ios_s descriptor;
	size_t size;
	size_t position;
	ios_mem_block_t last;
	ios_mem_block_t current;
	ios_mem_block_s first;
} ios_mem_s;

typedef ios_mem_s *ios_mem_t;

void ios_m_write(void *, void *, size_t);
void ios_m_read(void *, void *, size_t);
bool ios_m_eof(void *ms) {
	return ((ios_mem_t) ms)->position == ((ios_mem_t) ms)->size;
}
long long ios_m_tell(void *ms) {
	return ((ios_mem_t) ms)->position;
}
void ios_m_seek(void *ms, long long offset, int origin) {
	ios_mem_t m = (ios_mem_t) ms;
	switch(origin) {
	case IOS_SEEK_BEGIN:
		if(offset>0 && m->size>=offset) {
			m->position = offset;
			m->current = &m->first;
			while(offset>IOS_MEM_BLOCK_SIZE) {
				offset -= IOS_MEM_BLOCK_SIZE;
				m->current = m->current->next;
			}
			return;
		} else
			err_set(e_ios_error);
		break;
	case IOS_SEEK_CURRENT:
		if((offset+m->position >= 0) && (offset+m->position <= m->size)) {
			m->position += offset;
			if(offset>0) {
				while(offset>IOS_MEM_BLOCK_SIZE) {
					offset -= IOS_MEM_BLOCK_SIZE;
					m->current = m->current->next;
				}
			} else {
				offset = -offset;
				while(offset>IOS_MEM_BLOCK_SIZE) {
					offset -= IOS_MEM_BLOCK_SIZE;
					m->current = m->current->prev;
				}
			}
		} else
			err_set(e_ios_error);
		break;
	case IOS_SEEK_END:
		if(offset<0 && (-offset)<=m->size) {
			offset = -offset;
			m->position = m->size - offset;
			while(offset>IOS_MEM_BLOCK_SIZE) {
				offset -= IOS_MEM_BLOCK_SIZE;
				m->current = m->current->prev;
			}
			return;
		} else
			err_set(e_ios_error);
		break;
	default:
		err_set(e_ios_error);
	}
}
void ios_m_flush(void *ms) {}
void ios_m_close(void *ms) {
	if(ms) {
		ios_mem_block_t i = ((ios_mem_t) ms)->first.next;
		mem_free(ms);
		while(i) {
			ios_mem_block_t j = i;
			i = i->next;
			mem_free(j);
		}
	}
}

ios_t ios_mem_create() {
	ios_mem_t r = mem_alloc(sizeof(ios_mem_s));
	if(err())
		return 0;
	r->descriptor.data = r;
	r->size = 0;
	r->position = 0;
	r->current = r->last = &r->first;
	r->first.next = r->first.prev = 0;
	return &r->descriptor;
}


// Common functions

void ios_write(ios_t s, void *p, size_t sz) {
	err_reset();
	s->vtable->write(s->data, p, sz);
}
void ios_read(ios_t s, void * p, size_t sz) {
	err_reset();
	s->vtable->read(s->data, p, sz);
}
bool ios_eof(ios_t s) {
	err_reset();
	return s->vtable->eof(s->data);
}
long long ios_tell(ios_t s) {
	err_reset();
	return s->vtable->tell(s->data);
}
void ios_seek(ios_t s, long long pos, int origin) {
	err_reset();
	s->vtable->seek(s->data, pos, origin);
}
ios_t ios_close(ios_t s) {
	s->vtable->close(s->data);
	return 0;
}
