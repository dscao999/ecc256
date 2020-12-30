#ifndef MD5_DSCAO__
#define MD5_DSCAO__

typedef unsigned int wd32;
typedef unsigned char wd8;

struct md5_ctx {
	unsigned long blen;
	wd32 state[4];
	wd8 buf[64];
};

void md5_init(struct md5_ctx *ctx);
void md5_update(struct md5_ctx *ctx, const wd8 *msg, wd32 len);
void md5_exit(wd8 digest[16], struct md5_ctx *ctx);

#endif /* MD5_DSCAO__ */
