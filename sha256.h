/*
 *  SHA-3 256 bit message digest implementation, Dashi Cao, dscao999@hotmail.com, caods1@lenovo.com
 *
 */
#ifndef SHA256_DSCAO__
#define SHA256_DSCAO__
#include <stdlib.h>

#define SHA_START  0x02
#define SHA_END 0x01

#define SHA_BLOCK_LEN	64
#define SHA_DGST_LEN	8

struct sha256{
	unsigned int W[SHA_BLOCK_LEN];
	unsigned char pad_block[SHA_BLOCK_LEN];
};

static inline struct sha256 *sha256_init(void)
{
	return malloc(sizeof(struct sha256));
}

static inline void sha256_exit(struct sha256 *sha_handle)
{
	free(sha_handle);
}

void sha256_block(struct sha256 *hd, const unsigned char *buf,
		unsigned int *H, unsigned long len, int flag);

void sha256(struct sha256 *hd, const unsigned char *buf,
		unsigned long len, unsigned int *H);

#endif  /* SHA256_DSCAO__ */
