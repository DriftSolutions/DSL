#ifndef __MD5_H__
#define __MD5_H__

#define MD5_HASH_SIZE 16

struct md5_context {
	uint32 buf[4];
	uint32 bits[2];
	unsigned char in[64];
};

void md5_init(struct md5_context *ctx);
void md5_update(struct md5_context *ctx, const uint8 *input, uint32 length);
void md5_finish(struct md5_context *ctx, uint8 digest[16]);
void md5_transform(uint32 buf[4], uint32 const in[16]);

#endif // __MD5_H__
