#include "utf.h"
#include "str.h"
#include "mem.h"
#include "err.h"
#include <assert.h>

ERR_DEFINE(e_utf_conversion, "Invalid UTF string or encoding.", 0);

#define C_BETWEEN(p, c1, c2) (*((unsigned char *)(p))>=(c1) && *((unsigned char *)(p))<=(c2))
#define C_OFFSET(p,s) (((unsigned char*)(p))+(s))
#define C_LESS(p,c) (*((unsigned char *)(p))<(c))
#define C_TO_LONG(p, offset) ((unsigned long)(*((unsigned char *)((p)+(offset)))))

#define CHAR_LEN(p, enc) (((enc)==UTF_32_LE || (enc)==UTF_32_BE) ? 4 : (               \
		((enc)==UTF_16_BE) ? (C_BETWEEN((p), 0xD8, 0xDB)?4:2) : (                      \
			((enc)==UTF_16_LE) ? (C_BETWEEN(C_OFFSET((p),1), 0xD8, 0xDB)?4:2) : (      \
				C_LESS((p),0x80)? 1 : (                                                \
					C_LESS((p),0xE0) ? 2 : (                                           \
						C_LESS((p),0xF0) ? 3 : 4                                       \
					)                                                                  \
				)                                                                      \
			)                                                                          \
		)                                                                              \
	)                                                                                  \
)

#define CHAR_IS_LAST(p, enc) (							\
	((enc)==UTF_32_LE || (enc)==UTF_32_BE) ? (			\
		(*((long *)(p)))==0								\
	) : (												\
		((enc)==UTF_16_LE || (enc)==UTF_16_BE) ? (		\
			(*((short *)(p)))==0							\
		) : (											\
			(*((char *)(p)))==0							\
		)												\
	)													\
)

#define CHAR_TO_LONG(p, enc) (((enc)==UTF_32_BE) ? (													\
		(C_TO_LONG((p),0)<<24)|(C_TO_LONG((p),1)<<16)|(C_TO_LONG((p),2)<<8)|C_TO_LONG((p),3)			\
	) : (																								\
		(enc==UTF_32_LE) ? (																			\
			(C_TO_LONG((p),3)<<24)|(C_TO_LONG((p),2)<<16)|(C_TO_LONG((p),1)<<8)|C_TO_LONG((p),0)		\
		) : (																							\
			(enc==UTF_16_LE) ? (																		\
				C_BETWEEN(C_OFFSET((p),1),0xD8,0xDB) ? (												\
					(((C_TO_LONG((p),0) | (C_TO_LONG((p),1)<<8))-0xD800)<<10)							\
						+ ((C_TO_LONG((p),2) | (C_TO_LONG((p),3)<<8))-0xD800)							\
						+ 0x10000																		\
				) : (																					\
					C_TO_LONG((p),0) | (C_TO_LONG((p),1)<<8)											\
				)																						\
			) : (																						\
				(enc==UTF_16_BE) ? (																	\
					C_BETWEEN((p),0xD8,0xDB) ? (														\
						(((C_TO_LONG((p),1) | (C_TO_LONG((p),0)<<8))-0xD800)<<10)						\
							+ ((C_TO_LONG((p),3) | (C_TO_LONG((p),2)<<8))-0xD800)						\
							+ 0x10000																	\
					) : (																				\
						C_TO_LONG((p),1) | (C_TO_LONG((p),0)<<8)										\
					)																					\
				) : (																					\
					C_LESS((p),0x80) ? C_TO_LONG((p),0) : (												\
						C_LESS((p),0xE0) ? (															\
							((C_TO_LONG((p),0)&0x1F)<<6) + (C_TO_LONG((p),1)&0x3F)						\
						) : (																			\
							C_LESS((p),0xF0) ? ( /*3*/													\
								((C_TO_LONG((p),0)&0x0F)<<12) 											\
									| ((C_TO_LONG((p),1)&0x3F)<<6) 										\
									| (C_TO_LONG((p),2)&0x3F)											\
							) : ( /*4*/																	\
								((C_TO_LONG((p),0)&0x07)<<18) 											\
									| ((C_TO_LONG((p),1)&0x3F)<<12)										\
									| ((C_TO_LONG((p),2)&0x3F)<<6) 										\
									| (C_TO_LONG((p),2)&0x3F)											\
							)																			\
						)																				\
					)  																					\
				)																						\
			)																							\
		)																								\
	)																									\
)

#define UTF_WRITE1(dest, c1) ( \
	((*C_OFFSET(dest, 0))=c1), \
	C_OFFSET(dest, 1)          \
)

#define UTF_WRITE2(dest, c1, c2) (      \
	((*C_OFFSET(dest, 0))=c1),          \
	((*C_OFFSET(dest, 1))=c2),          \
	C_OFFSET(dest, 2)                   \
)

#define UTF_WRITE3(dest, c1, c2, c3) (  \
	((*C_OFFSET(dest, 0))=c1),		 	\
	((*C_OFFSET(dest, 1))=c2),			\
	((*C_OFFSET(dest, 2))=c3),			\
	C_OFFSET(dest, 3)					\
)

#define UTF_WRITE4(dest, c1, c2, c3, c4) (		\
	((*C_OFFSET(dest, 0))=c1), 					\
	((*C_OFFSET(dest, 1))=c2),					\
	((*C_OFFSET(dest, 2))=c3),					\
	((*C_OFFSET(dest, 3))=c4),					\
	C_OFFSET(dest, 4)							\
)

#define LONG_TO_UTF(c, dest, enc)									\
	((enc)==UTF_32_LE) ? (											\
		UTF_WRITE4(													\
			(dest),													\
			(c)&0xFF,												\
			((c)>>8)&0xFF,											\
			((c)>>16)&0xFF,											\
			((c)>>24)&0xFF											\
		)															\
	) : (															\
		((enc)==UTF_32_BE) ? (										\
			UTF_WRITE4(												\
				(dest), 											\
				((c)>>24)&0xFF,										\
				((c)>>16)&0xFF, 									\
				((c)>>8)&0xFF, 										\
				(c)&0xFF											\
			)														\
		) : (														\
			((enc)==UTF_16_LE) ? (									\
				UTF_WRITE2((dest), (c)&0xFF, ((c)>>8)&0xFF)			\
			) : (													\
				((enc)==UTF_16_BE) ? (								\
					UTF_WRITE2((dest), ((c)>>8)&0xFF, (c)&0xFF)		\
				) : (												\
					((c)<=0x7f) ? (									\
						UTF_WRITE1((dest), (c))						\
					) : (											\
						((c)<=0x07FF) ? (							\
							UTF_WRITE2(								\
								(dest), 							\
								(((c)>>6)&0x1F)|0xC0, 				\
								((c)&0x3F) | 0x80					\
							)										\
						) : (										\
							((c)<=0xFFFF) ? (						\
								UTF_WRITE3(							\
									(dest),							\
									(((c)>>12)&0x0F)|0xE0,			\
									(((c)>>6)&0x3F) | 0x80,			\
									((c)&0x3F) | 0x80				\
								)									\
							) : (									\
								UTF_WRITE4(							\
									(dest), 						\
									(((c)>>18)&0x07)|0xF0, 			\
									(((c)>>12)&0x3F) | 0x80, 		\
									(((c)>>6)&0x3F) | 0x80, 		\
									((c)&0x3F) | 0x80				\
								)									\
							)										\
						)											\
					)												\
				)													\
			)														\
		)															\
	)


size_t utf_length(str_t s, int enc) {
	size_t res = 0;
	str_it_t i = str_begin(s);
	str_it_t e = str_end(s);
	while(i<e) {
		int cl = CHAR_LEN(i, enc);
		res++;
		i += cl;
		if(i>e)
			err_throw(e_utf_conversion);
	}
	return res;
}

str_it_t utf_next(str_it_t i, int enc) {
	return i + CHAR_LEN(i, enc);
}

str_t utf_convert(heap_t h, str_t src, int src_enc, int dest_enc) {
	str_t res = str_create(h, utf_length(src, src_enc)*4);
	str_it_t src_i = str_begin(src),
		src_e = str_end(src),
		res_i = str_begin(res);
	while(src_i<src_e) {
		long c = CHAR_TO_LONG(src_i, src_enc);
		src_i += CHAR_LEN(src_i, src_enc);
		res_i = LONG_TO_UTF(c, res_i, dest_enc);
	}
	res->length = res_i - str_begin(res);
	*res_i = 0;
	return res;
}
