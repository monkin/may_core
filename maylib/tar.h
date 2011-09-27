
#ifndef MAY_TAR_H
#define MAY_TAR_H

#include "stream.h"
#include "str.h"

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

typedef struct {

} tar_s;

typedef tar_s *tar_t;

tar_t tar_create(ios_t);
ios_t tar_find(tar_t, str_t fname);
str_t tar_get(tar_t, str_t fname);
void tar_put(tar_t, str_t fname, str_t content);
void tar_putf(tar_t, str_t fname, str_t path);
void tar_puts(tar_t, str_t fname, ios_t);
tar_t tar_delete(tar_t);


#endif /* MAY_TAR_H */

