#ifndef MAY_FLOADER_H
#define MAY_FLOADER_H

#include "tar.h"
#include "stream.h"
#include "str.h"

typedef struct {
	void *data;
	ios_t (*get_stream)(void *, str_t);
	str_t (*get_str)(void *, heap_t, str_t);
} floader_s;

typedef floader_s *floader_t;

floader_t floader_create_tar(heap_t, tar_t, str_t);
floader_t floader_create_dir(heap_t, str_t);

ios_t floader_get_stream(floader_t, str_t);
str_t floader_get_str(floader_t, heap_t, str_t);
ios_t floader_get_stream_cs(floader_t, const char *);
str_t floader_get_str_cs(floader_t, heap_t, const char *);


#endif /* MAY_FLOADER_H */