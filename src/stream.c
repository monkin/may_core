
#include "stream.h"
#include "mem.h"
#include <string.h>

ERR_DEFINE(e_ios_error, "IO error.", 0);
ERR_DEFINE(e_ios_invalid_mode, "Invalid file open mode.", e_ios_error);

/* Range stream */

typedef struct {
	ios_s self;
	ios_t stream;
	long long interval[2];
} ios_r_s;

typedef ios_r_s *ios_r_t;

static long long ios_r_tell(void *f);

static size_t ios_r_write(void *f, const void *dt, size_t sz, size_t cnt) {
	ios_r_t s = (ios_r_t) f;
	long long spos = ios_r_tell(f);
	long long ssz = s->interval[1] - s->interval[0];
	if(ssz<(cnt*sz))
		cnt = ssz/sz;
	return ios_write_n(s->stream, dt, sz, cnt);
}
static size_t ios_r_read(void *f, void *dt, size_t sz, size_t cnt) {
	ios_r_t s = (ios_r_t) f;
	long long spos = ios_r_tell(f);
	long long ssz = s->interval[1] - s->interval[0];
	if(ssz<(cnt*sz))
		cnt = ssz/sz;
	return ios_read_n(s->stream, dt, sz, cnt);
}
static bool ios_r_eof(void *f) {
	ios_r_t s = (ios_r_t) f;
	return ios_tell(&s->self) == (s->interval[1] - s->interval[0]);
}
static long long ios_r_tell(void *f) {
	ios_r_t s = (ios_r_t) f;
	long long p = ios_tell(s->stream);
	if(p<s->interval[0])
		return 0;
	else if(p>s->interval[1])
		return s->interval[1];
	else
		return p - s->interval[0];
}
static void ios_r_seek(void *f, long long pos, int origin) {
	ios_r_t s = (ios_r_t) f;
	long long p = ios_tell(s->stream);
	switch(origin) {
	case IOS_SEEK_BEGIN:
		pos += s->interval[0];
		break;
	case IOS_SEEK_CURRENT:
		pos += ios_tell(&s->self);
		break;
	case IOS_SEEK_END:
		pos += s->interval[0] + s->interval[1];
		break;
	}
	if(pos < s->interval[0] || pos > (s->interval[0]+s->interval[1])) {
		err_throw(e_ios_error);
	} else
		ios_seek(s->stream, pos, IOS_SEEK_BEGIN);
}
static void ios_r_flush(void *f) {
	ios_flush(((ios_r_t) f)->stream);
}
static void ios_r_close(void *f) {}

static ios_table_s ios_r_vtable = { ios_r_write, ios_r_read, ios_r_eof, ios_r_tell, ios_r_seek, ios_r_flush, ios_r_close };

ios_t ios_range_create(heap_t h, ios_t stream, long long begin, long long end) {
	ios_r_t r = heap_alloc(h, sizeof(ios_r_s));
	r->self.data = r;
	r->self.vtable = &ios_r_vtable;
	r->stream = stream;
	r->interval[0] = begin;
	r->interval[1] = end;
	return &r->self;
}

/* File streams. */

typedef struct {
	FILE *file;
	void *block;
} ios_f_s;

typedef ios_f_s *ios_f_t;

static size_t ios_f_write(void *f, const void *dt, size_t sz, size_t cnt) {
	return fwrite(dt, sz, cnt, ((ios_f_t) f)->file);
}
static size_t ios_f_read(void *f, void *dt, size_t sz, size_t cnt) {
	return fread(dt, sz, cnt, ((ios_f_t) f)->file);
}
static bool ios_f_eof(void *f) {
	return feof(((ios_f_t) f)->file);
}
static long long ios_f_tell(void *f) {
	return ftello64(((ios_f_t) f)->file);
}
static void ios_f_seek(void *f, long long pos, int origin) {
	if(0!=fseeko64(((ios_f_t) f)->file, pos, origin))
		err_throw(e_ios_error);
}
static void ios_f_flush(void *f) {
	fflush(((ios_f_t) f)->file);
}
static void ios_f_close(void *f) {
	fclose(((ios_f_t) f)->file);
	mem_free(((ios_f_t) f)->block);
}

static ios_table_s ios_rf_vtable = { 0, ios_f_read, ios_f_eof, ios_f_tell, ios_f_seek, 0, ios_f_close };
static ios_table_s ios_wf_vtable = { ios_f_write, 0, ios_f_eof, ios_f_tell, ios_f_seek, ios_f_flush, ios_f_close };
static ios_table_s ios_rwf_vtable = { ios_f_write, ios_f_read, ios_f_eof, ios_f_tell, ios_f_seek, ios_f_flush, ios_f_close };

ios_t ios_file_create(str_t s, ios_mode_t mode) {
	const char *md = 0;
	switch(mode) {
	case IOS_MODE_R: md = "r"; break;
	case IOS_MODE_W: md = "w"; break;
	case IOS_MODE_RP: md = "r+"; break;
	case IOS_MODE_WP: md = "w+"; break;
	default:
		err_throw(e_ios_invalid_mode);
	}
	FILE *f = fopen(str_begin(s), md);
	if(!f)
		err_throw(e_ios_error);
	char *mem = 0;
	err_try {
		mem = mem_alloc(sizeof(ios_s) + sizeof(ios_f_s));
	} err_catch {
		fclose(f);
		err_throw_down();
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
	fclose(((ios_f_t) f)->file);
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

static size_t ios_m_write(void *ms, const void *dt, size_t sz, size_t cnt) {
	ios_mem_t m = (ios_mem_t) ms;
	size_t fsz = sz*cnt;
	while(m->block_count*IOS_MEM_BLOCK_SIZE < m->position+fsz) {
		m->last->next = mem_alloc(sizeof(ios_mem_block_s));
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
		if(sw>fsz)
			sw = fsz;
		memcpy(m->current->data + boff, dt, sw);
		fsz -= sw;
		dt = ((char *) dt) + sw;
		m->position += sw;
		if(m->position>m->size)
			m->size = m->position;
		if(fsz!=0)
			m->current = m->current->next;
	}
	return succ_cnt;
}
static size_t ios_m_read(void *ms, void *dt, size_t sz, size_t cnt) {
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
static bool ios_m_eof(void *ms) {
	return ((ios_mem_t) ms)->position == ((ios_mem_t) ms)->size;
}
static long long ios_m_tell(void *ms) {
	return ((ios_mem_t) ms)->position;
}
static void ios_m_seek(void *ms, long long offset, int origin) {
	ios_mem_t m = (ios_mem_t) ms;
	switch(origin) {
	case IOS_SEEK_BEGIN:
		if(offset>=0 && m->size>=offset) {
			m->position = offset;
			m->current = &m->first;
			while(offset>IOS_MEM_BLOCK_SIZE) {
				offset -= IOS_MEM_BLOCK_SIZE;
				m->current = m->current->next;
			}
			return;
		} else
			err_throw(e_ios_error);
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
			return;
		} else
			err_throw(e_ios_error);
	case IOS_SEEK_END:
		if(offset<=0 && (-offset)<=m->size) {
			offset = -offset;
			m->position = m->size - offset;
			while(offset>IOS_MEM_BLOCK_SIZE) {
				offset -= IOS_MEM_BLOCK_SIZE;
				m->current = m->current->prev;
			}
			return;
		} else
			err_throw(e_ios_error);
	default:
		err_throw(e_ios_error);
	}
}
static void ios_m_flush(void *ms) {}
static void ios_m_close(void *ms) {
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

size_t ios_write_n(ios_t s, const void *p, size_t sz, size_t cnt) {
	return s->vtable->write(s->data, p, sz, cnt);
}
size_t ios_read_n(ios_t s, void * p, size_t sz, size_t cnt) {
	return s->vtable->read(s->data, p, sz, cnt);
}
void ios_write(ios_t s, const void *p, size_t sz) {
	if(!s->vtable->write(s->data, p, sz, 1))
		err_throw(e_ios_error);
}
void ios_read(ios_t s, void * p, size_t sz) {
	if(!s->vtable->read(s->data, p, sz, 1))
		err_throw(e_ios_error);
}
bool ios_eof(ios_t s) {
	return s->vtable->eof(s->data);
}
long long ios_tell(ios_t s) {
	return s->vtable->tell(s->data);
}
void ios_seek(ios_t s, long long pos, int origin) {
	s->vtable->seek(s->data, pos, origin);
}
void ios_flush(ios_t s) {
	s->vtable->flush(s->data);
}
ios_t ios_close(ios_t s) {
	if(s)
		s->vtable->close(s->data);
	return 0;
}

