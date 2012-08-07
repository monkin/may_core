
#include "floader.h"

static str_t floader_join_path(heap_t h, str_t p1, str_t p2) {
	if(p1 ? str_length(p1) : false) {
		if(p2 ? str_length(p2) : false) {
			if(*(str_end(p1)-1) == '/')
				return str_cat(h, p1, str_interval(h, str_begin(p2)+1, str_end(p2)));
			else if(*str_begin(p2) == '/')
				return str_cat(h, p1, p2);
			else {
				sbuilder_t sb = sbuilder_create(h);
				sbuilder_append(sb, p1);
				sbuilder_append_cs(sb, "/");
				sbuilder_append(sb, p2);
				return sbuilder_get(h, sb);
			}
		} else
			return str_clone(h, p1);
	} else
		return str_clone(h, p2);
}

typedef struct {
	str_t path;
	tar_t tar;
} floader_tar_s;

typedef floader_tar_s *floader_tar_t;

typedef struct {
	str_t path;
	floader_t floader;
} floader_sub_s;

typedef floader_sub_s *floader_sub_t;

static ios_t floader_sub_get_stream(void *dt, str_t path) {
	heap_t h = heap_create(1024);
	ios_t r;
	err_try {
		r = floader_get_stream(((floader_sub_t)dt)->floader, floader_join_path(h, ((floader_sub_t)dt)->path, path));
		h = heap_delete(h);
	} err_catch {
		heap_delete(h);
		err_throw_down();
	}
	if(r)
		return r;
	else
		err_throw(e_ios_error);
}
static str_t floader_sub_get_str(void *dt, heap_t h, str_t path) {
	return floader_get_str(((floader_sub_t)dt)->floader, h, floader_join_path(h, ((floader_sub_t)dt)->path, path));
}

static ios_t floader_tar_get_stream(void *dt, str_t path) {
	heap_t h = heap_create(1024);
	ios_t r;
	err_try {
		r = tar_get_stream(((floader_tar_t)dt)->tar, floader_join_path(h, ((floader_tar_t)dt)->path, path));
		h = heap_delete(h);
	} err_catch {
		heap_delete(h);
		err_throw_down();
	}
	if(r)
		return r;
	else
		err_throw(e_ios_error);
}
static str_t floader_tar_get_str(void *dt, heap_t h, str_t path) {
	str_t r = tar_get(((floader_tar_t)dt)->tar, h, floader_join_path(h, ((floader_tar_t)dt)->path, path));
	if(r)
		return r;
	else
		err_throw(e_ios_error);
}
static ios_t floader_dir_get_stream(void *dt, str_t path) {
	heap_t h = heap_create(1024);
	err_try {
		ios_t r = ios_file_create(floader_join_path(h, (str_t)dt, path), IOS_MODE_R);
		h = heap_delete(h);
		return r;
	} err_catch {
		heap_delete(h);
		err_throw_down();
	}
}
static str_t floader_dir_get_str(void *dt, heap_t h, str_t path) {
	str_t r = 0;
	ios_t s = ios_file_create(floader_join_path(h, (str_t)dt, path), IOS_MODE_R);
	ios_seek(s, 0, IOS_SEEK_END);
	r = str_create(h, ios_tell(s));
	ios_seek(s, 0, IOS_SEEK_BEGIN);
	ios_read(s, str_begin(r), str_length(r));
	return r;
}

floader_t floader_create_tar(heap_t h, tar_t tar, str_t path) {
	floader_t r = heap_alloc(h, sizeof(floader_s));
	floader_tar_t data = r->data = heap_alloc(h, sizeof(floader_tar_s));
	data->path = str_clone(h, path);
	data->tar = tar;
	r->get_stream = floader_tar_get_stream;
	r->get_str = floader_tar_get_str;
	return r;
}

floader_t floader_create_dir(heap_t h, str_t s) {
	floader_t r = heap_alloc(h, sizeof(floader_s));
	r->data = str_clone(h, s);
	r->get_stream = floader_dir_get_stream;
	r->get_str = floader_dir_get_str;
}

floader_t floader_create_sub(heap_t h, floader_t fl, str_t s) {
	floader_t r = heap_alloc(h, sizeof(floader_s));
	floader_sub_t data = r->data = heap_alloc(h, sizeof(floader_sub_s));
	data->path = str_clone(h, s);
	data->floader = fl;
	r->get_stream = floader_sub_get_stream;
	r->get_str = floader_sub_get_str;
	return r;
}

ios_t floader_get_stream(floader_t fl, str_t name) {
	return fl->get_stream(fl->data, name);
}
str_t floader_get_str(floader_t fl, heap_t h, str_t name) {
	return fl->get_str(fl->data, h, name);
}
ios_t floader_get_stream_cs(floader_t fl, const char *cnm) {
	heap_t h = heap_create(1024);
	err_try {
		ios_t r = fl->get_stream(fl->data, str_from_cs(h, cnm));
		h = heap_delete(h);
		return r;
	} err_catch {
		heap_delete(h);
		err_throw_down();
	}
}
str_t floader_get_str_cs(floader_t fl, heap_t h, const char *cnm) {
	return fl->get_str(fl->data, h, str_from_cs(h, cnm));
}