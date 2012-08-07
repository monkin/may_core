
#ifndef MAY_STREAM_H
#define MAY_STREAM_H

#include "str.h"
#include <stdbool.h>
#include <stdio.h>

ERR_DECLARE(e_ios_error);
ERR_DECLARE(e_ios_invalid_mode);

#define IOS_SEEK_BEGIN SEEK_SET
#define IOS_SEEK_CURRENT SEEK_CUR
#define IOS_SEEK_END SEEK_END

enum {
	/* If you need some io buffer and don't now prefered size - use this constant. */
	IOS_DEFAULT_BUFFER_SIZE = 4*1024
};

typedef struct {
	size_t (* write)(void *, const void *, size_t, size_t);
	size_t (* read)(void *, void *, size_t, size_t);
	bool (* eof)(void *);
	long long (* tell)(void *);
	void (* seek)(void *, long long, int);
	void (* flush)(void *);
	void (* close)(void *);
} ios_table_s;

typedef ios_table_s *ios_table_t;

typedef struct {
	void *data;
	ios_table_t vtable;
} ios_s;

typedef ios_s *ios_t;

ios_t ios_std_in();
ios_t ios_std_out();
ios_t ios_std_err();

typedef enum {
	IOS_MODE_R = 1, /* r */
	IOS_MODE_W = 2, /* w */
	IOS_MODE_TRUNC = 4,
	IOS_MODE_RW = IOS_MODE_R | IOS_MODE_W, /* r+ */
	IOS_MODE_RP = IOS_MODE_R | IOS_MODE_W, /* r+ */
	IOS_MODE_WP = IOS_MODE_R | IOS_MODE_W | IOS_MODE_TRUNC /* w+ */
} ios_mode_t;

ios_t ios_mem_create();
str_t ios_mem_to_string(ios_t, heap_t); // !!! Return zero ended str_t

ios_t ios_file_create(str_t, ios_mode_t);

/* Calling "ios_close" for ios_range not necessary */
ios_t ios_range_create(heap_t, ios_t, long long position, long long size);

size_t ios_write_n(ios_t, const void *, size_t, size_t);
size_t ios_read_n(ios_t, void *, size_t, size_t);
void ios_write(ios_t, const void *, size_t);
#define ios_write_cs(ios, s) { const char *may_ios_str = (s); ios_write((ios), may_ios_str, strlen(may_ios_str)); }
void ios_read(ios_t, void *, size_t);
bool ios_eof(ios_t);
long long ios_tell(ios_t);
void ios_seek(ios_t, long long, int);
void ios_flush(ios_t);
ios_t ios_close(ios_t);

#endif /* MAY_STREAM_H */

