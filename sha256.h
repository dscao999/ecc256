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
	unsigned int H[SHA_DGST_LEN];
	unsigned int W[SHA_BLOCK_LEN];
	unsigned char pad_block[SHA_BLOCK_LEN];
};

void sha256_reset(struct sha256 *sha);

static inline struct sha256 *sha256_init(void)
{
	struct sha256 *sha;
	sha = malloc(sizeof(struct sha256));
	sha256_reset(sha);
	return sha;
}

static inline void sha256_exit(struct sha256 *sha)
{
	free(sha);
}

void sha256_block(struct sha256 *hd, const unsigned char *buf,
		unsigned long len, int flag);

void sha256(struct sha256 *hd, const unsigned char *buf, unsigned long len);

#endif  /* SHA256_DSCAO__ */
