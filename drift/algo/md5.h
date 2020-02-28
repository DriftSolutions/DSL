#ifndef __MD5_H__
#define __MD5_H__

#include <drift/rwops.h>
#define MD5_HASH_SIZE 16

struct md5_context {
	uint32 buf[4];
	uint32 bits[2];
	unsigned char in[64];
};

DSL_API int DSL_CC md5(unsigned char * data, unsigned int datalen, char * outbuf);
#define md5c(x,y,z) md5((unsigned char *)x,y,z)
DSL_API int DSL_CC md5file(const char * fn, char * outbuf);
DSL_API int DSL_CC md5file_fp(FILE * fp, char * outbuf);
DSL_API int DSL_CC md5file_rw(TITUS_FILE * fp, char * outbuf);

//if you want to do it manually
#define md5_starts md5_init // compatibility with older code
DSL_API void DSL_CC md5_init(struct md5_context *ctx);
DSL_API void DSL_CC md5_update(struct md5_context *ctx, const uint8 *input, uint32 length);
DSL_API void DSL_CC md5_finish(struct md5_context *ctx, uint8 digest[16]);


#endif // __MD5_H__
