#include "str.h"
#include "err.h"
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define INT_BUFFER_LEN 64
#define DOUBLE_BUFFER_LEN 128

ERR_DEFINE(e_str_format, "Invalid string format.", 0);

str_t str_create(heap_t h, size_t sz) {
	assert(h);
	str_t r = heap_alloc(h, sizeof(may_str_s) + sz + 1);
	r->data = ((char *)r) + sizeof(may_str_s);
	r->length = sz;
	r->data[sz] = 0;
	return r;
}

str_t str_from_cs(heap_t h, const char *s) {
	assert(h);
	str_t r = str_create(h, strlen(s));
	memcpy(r->data, s, r->length);
	return r;
}

str_t str_from_bin(heap_t h, const void *s, size_t sz) {
	assert(h);
	str_t r = str_create(h, sz);
	memcpy(r->data, s, sz);
	return r;
}

str_t str_from_int(heap_t h, long long i) {
	assert(h);
	str_t r = str_create(h, INT_BUFFER_LEN);
	r->length = snprintf(r->data, INT_BUFFER_LEN + 1, "%lli", i);
	if(r->length > INT_BUFFER_LEN) {
		r = str_create(h, r->length);
		if(r)
			r->length = snprintf(r->data, r->length + 1, "%lli", i);
	}
	return r;
}

str_t str_from_double(heap_t h, double d) {
	assert(h);
	str_t r = str_create(h, DOUBLE_BUFFER_LEN);
	r->length = snprintf(r->data, DOUBLE_BUFFER_LEN + 1, "%g", d);
	if(r->length > DOUBLE_BUFFER_LEN) {
		r = str_create(h, r->length+1);
		if(r)
			r->length = snprintf(r->data, r->length + 1, "%g", d);
	}
	return r;
}

double str_to_double(str_t s) {
	double r;
	int f, n;
	if(*str_end(s)) {
		if(str_length(s)<DOUBLE_BUFFER_LEN) {
			char buff[DOUBLE_BUFFER_LEN];
			memcpy(buff, str_begin(s), str_length(s));
			buff[str_length(s)] = '\0';
			f = sscanf(buff, "%lf%n", &r, &n);
		} else {
			char *buff = mem_alloc(str_length(s)+1);
			memcpy(buff, str_begin(s), str_length(s));
			buff[str_length(s)] = '\0';
			f = sscanf(buff, "%lf%n", &r, &n);
			free(buff);
		}
	} else
		f = sscanf(str_begin(s), "%lf%n", &r, &n);
	if(f!=EOF && n==(str_end(s)-str_begin(s)))
		return r;
	else
		err_throw(e_str_format);
}

long long str_to_int(str_t s) {
	long long r;
	int f, n;
	if(*str_end(s)) {
		if(str_length(s)<INT_BUFFER_LEN) {
			char buff[INT_BUFFER_LEN];
			memcpy(buff, str_begin(s), str_length(s));
			buff[str_length(s)] = '\0';
			f = sscanf(buff, "%lld%n", &r, &n);
		} else {
			char *buff = mem_alloc(str_length(s)+1);
			memcpy(buff, str_begin(s), str_length(s));
			buff[str_length(s)] = '\0';
			f = sscanf(buff, "%lld%n", &r, &n);
			free(buff);
		}
	} else
		f = sscanf(str_begin(s), "%lld%n", &r, &n);
	if(f!=EOF && n==(str_end(s)-str_begin(s)))
		return r;
	else
		err_throw(e_str_format);
}

str_t str_cat(heap_t h, str_t s1, str_t s2) {
	assert(s1 && s2);
	str_t r = str_create(h, s1->length + s2->length);
	memcpy(r->data, s1->data, s1->length);
	memcpy(r->data+s1->length, s2->data, s2->length);
	return r;
}

str_it_t str_end(str_t s) {
	return str_begin(s) + s->length;
}

str_t str_interval(heap_t h, str_it_t ps1, str_it_t ps2) {
	str_t res;
	assert(h);
	assert((ps1 && ps2) || (!ps1 && !ps2));
	res = heap_alloc(h, sizeof(may_str_s));
	res->length = ps2-ps1;
	res->data = ps1;
	return res;
}

str_t str_clone(heap_t h, str_t s) {
	assert(s);
	str_t r = str_create(h, s->length);
	memcpy(r->data, s->data, s->length);
	return r;
}

sb_t sb_create(heap_t h) {
	sb_t r = (sb_t) heap_alloc(h, sizeof(sb_s));
	r->length = 0;
	r->heap = h;
	r->first = r->last = 0;
	return r;
}

static sb_t sb_append_node(sb_t sb, sb_item_t i) {
	sb->length += i->str ? i->str->length : i->sb->length;
	if(sb->first)
		sb->last = sb->last->next = i;
	else
		sb->first = sb->last = i;
	return sb;
}

static sb_t sb_preppend_node(sb_t sb, sb_item_t i) {
	sb->length += i->str ? i->str->length : i->sb->length;
	i->next = sb->first;
	if(sb->first)
		sb->first = i;
	else
		sb->first = sb->last = i;
	return sb;
}

static sb_item_t sb_create_node(sb_t sb, str_t data_str, sb_t data_sb) {
	sb_item_t i = heap_alloc(sb->heap, sizeof(sb_item_s));
	i->str = data_str;
	i->sb = data_sb;
	i->next = 0;
	return i;
}

sb_t sb_append(sb_t sb, str_t s) {
	return sb_append_node(sb, sb_create_node(sb, s, 0));
}

sb_t sb_append_sb(sb_t sb, sb_t data_sb) {
	return sb_append_node(sb, sb_create_node(sb, 0, data_sb));
}

sb_t sb_preppend(sb_t sb, str_t s) {
	return sb_preppend_node(sb, sb_create_node(sb, s, 0));
}

sb_t sb_preppend_sb(sb_t sb, sb_t data_sb) {
	return sb_preppend_node(sb, sb_create_node(sb, 0, data_sb));
}

static str_it_t sb_write(sb_t sb, str_it_t s_position, str_it_t s_end) {
	sb_item_t i;
	for(i=sb->first; i; i=i->next) {
		if(i->str) {
			assert((s_end - s_position) >= i->str->length);
			memcpy(s_position, i->str->data, i->str->length);
			s_position += i->str->length;
		} else {
			assert((s_end - s_position) >= i->sb->length);
			s_position = sb_write(i->sb, s_position, s_end);
		}
	}
	return s_position;
}

str_t sb_get(heap_t h, sb_t sb) {
	str_t r = str_create(h, sb->length);
	str_it_t s_end = sb_write(sb, str_begin(r), str_end(r));
	assert(s_end == str_end(r));
	return r;
}

int str_compare(str_t s1, str_t s2) {
	return str_compare_bin(s1, s2->data, s2->length);
}

int str_compare_cs(str_t s1, const char *s2) {
	return str_compare_bin(s1, s2, strlen(s2));
}

int str_compare_bin(str_t s1, const void *data, size_t sz) {
	int cmp_res = memcmp(s1->data, data, s1->length<sz ? s1->length : sz);
	if(cmp_res!=0)
		return cmp_res<0 ? -1 : 1;
	else if(s1->length==sz)
		return 0;
	else
		return s1->length>sz ? 1 : -1;
}

int str_equal(str_t s1, str_t s2) {
	assert(s1 && s2);
	if(s1->length!=s2->length)
		return 0;
	else
		return !str_compare(s1, s2);
}


