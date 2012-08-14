#ifndef MAY_UTF_H
#define MAY_UTF_H

#include "str.h"
#include "heap.h"
#include "err.h"

ERR_DECLARE(e_utf_conversion);

enum {
	UTF_8 = 1,
	UTF_16_LE = 2,
	UTF_16_BE = 3,
	UTF_32_LE = 4,
	UTF_32_BE = 5
};

str_t utf_convert(heap_t h, str_t src, int src_enc, int dest_enc);
size_t utf_length(str_t s, int enc);
str_it_t utf_next(str_it_t, int enc);

#endif /* MAY_UTF_H */


