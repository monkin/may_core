
#include "stream.h"
#include "mem.h"
#include <string.h>

ERR_DEFINE(e_ios_error, "IO error.", 0);
ERR_DEFINE(e_ios_invalid_mode, "Invalid file open mode.", e_ios_error);

/* File streams. */

typedef struct {
	FILE *file;
	void *block;
} ios_f_s;

typedef ios_f_s *ios_f_t;

static size_t ios_f_write(void *f, const void *dt, size_t sz, size_t cnt) {
	size_t r = fwrite(dt, sz, cnt, ((ios_f_t) f)->file);
	if(r!=cnt)
		err_set(e_ios_error);
	return r;
}
static size_t ios_f_read(void *f, void *dt, size_t sz, size_t cnt) {
	size_t r = fread(dt, sz, cnt, ((ios_f_t) f)->file);
	if(r!=cnt)
		err_set(e_ios_error);
	return r;
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
	fflush(((ios_f_t) f)->file);
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
	return &ios_std_outs;
}
ios_t ios_std_err() {
	ios_std_errfs.file = stderr;
	return &ios_std_errs;
}

/* Memory streams */

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
	size_t block_count;
	ios_mem_block_t last;
	ios_mem_block_t current;
	ios_mem_block_s first;
} ios_mem_s;

typedef ios_mem_s *ios_mem_t;

size_t ios_m_write(void *ms, const void *dt, size_t sz, size_t cnt) {
	ios_mem_t m = (ios_mem_t) ms;
	size_t fsz = sz*cnt;
	while(m->block_count*IOS_MEM_BLOCK_SIZE < m->position+fsz) {
		m->last->next = mem_alloc(sizeof(ios_mem_block_s));
		if(err()) {
			err_clear();
			break;
		}
		m->last->next->prev = m->last;
		m->last->next->next = 0;
		m->last = m->last->next;
		m->block_count++;
	}
	size_t succ_cnt = (m->block_count*IOS_MEM_BLOCK_SIZE - m->position) / sz;
	if(succ_cnt>cnt)
		succ_cnt = cnt;
	while(fsz>0) {
		size_t boff = m->position % IOS_MEM_BLOCK_SIZE;
		size_t sw = IOS_MEM_BLOCK_SIZE-boff;
		memcpy(m->current->data + boff, dt, sw);
		fsz -= sw;
		dt = ((char *) dt) + sw;
		m->position += sw;
		if(m->position>m->size)
			m->size = m->position;
		if(fsz!=0)
			m->current = m->current->next;
	}
	if(succ_cnt!=cnt)
		err_set(e_out_of_memory);
	return succ_cnt;
}
size_t ios_m_read(void *ms, void *dt, size_t sz, size_t cnt) {
	ios_mem_t m = (ios_mem_t) ms;
	if(sz*cnt+m->position > m->size)
		cnt = (m->size - m->position) / sz;
	sz *= cnt;
	char *i = dt;
	while(sz) {
		size_t boff = m->position % IOS_MEM_BLOCK_SIZE;
		size_t sr = IOS_MEM_BLOCK_SIZE - boff;
		if(sz >= sr) {
			memcpy(dt, m->current->data+boff, sr);
			m->position += sr;
			if(m->current->next)
				m->current = m->current->next;
			dt = ((char *) dt) + sr;
			sz -= sr;
		} else {
			memcpy(dt, m->current->data+boff, sz);
			m->position += sz;
			dt = ((char *) dt) + sz;
			sz = 0;
		}
	}
	return cnt;
}
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

static ios_table_s ios_m_vtable = { ios_m_write, ios_m_read, ios_m_eof, ios_m_tell, ios_m_seek, ios_m_flush, ios_m_close };

ios_t ios_mem_create() {
	ios_mem_t r = mem_alloc(sizeof(ios_mem_s));
	if(err())
		return 0;
	r->descriptor.data = r;
	r->descriptor.vtable = &ios_m_vtable;
	r->size = 0;
	r->position = 0;
	r->current = r->last = &r->first;
	r->first.next = r->first.prev = 0;
	r->block_count = 1;
	return &r->descriptor;
}

str_t ios_mem_to_string(ios_t ms, heap_t h) {
	ios_mem_t m = (ios_mem_t) ms;
	size_t 	sz = m->size;
	str_t r = str_create(h, sz);
	if(err())
		return 0;
	str_it_t sp = str_begin(r);
	ios_mem_block_t i = &m->first;
	while(sz) {
		size_t cps = sz>=IOS_MEM_BLOCK_SIZE ? IOS_MEM_BLOCK_SIZE : sz;
		memcpy(sp, i->data, cps);
		sp += cps;
		sz -= cps;
		i = i->next;
	}
	return r;
}

/* Common functions */

size_t ios_write(ios_t s, const void *p, size_t sz, size_t cnt) {
	err_reset();
	return s->vtable->write(s->data, p, sz, cnt);
}
size_t ios_read(ios_t s, void * p, size_t sz, size_t cnt) {
	err_reset();
	return s->vtable->read(s->data, p, sz, cnt);
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

