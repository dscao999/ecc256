#ifndef SHA256_DSCAO__
#define SHA256_DSCAO__
#include <stdlib.h>

#define SHA_START  0x02
#define SHA_END 0x01

#define SHA_BLOCK_LEN	64

struct sha256_handle {
	unsigned int W[64];
	unsigned char pad_block[SHA_BLOCK_LEN];
};

static inline struct sha256_handle *sha256_init(void)
{
	return malloc(sizeof(struct sha256_handle));
}

static inline void sha256_exit(struct sha256_handle *sha_handle)
{
	free(sha_handle);
}

void sha256_block(struct sha256_handle *hd, const unsigned char buf[64],
		unsigned int H[8], unsigned long len, int flag);

void sha256(struct sha256_handle *hd, const unsigned char *buf,
		unsigned long len, unsigned H[8]);

#endif  /* SHA256_DSCAO__ */
