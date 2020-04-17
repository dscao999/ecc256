/*
 *  SHA-3 256 bit message digest implementation, Dashi Cao, dscao999@hotmail.com, caods1@lenovo.com
 *
 */
#ifndef SHA256_DSCAO__
#define SHA256_DSCAO__
#include <stdio.h>
#include <stdlib.h>
#include "loglog.h"

#define SHA_BLOCK_LEN	64
#define SHA_DGST_LEN	32

struct sha256{
	unsigned int H[SHA_DGST_LEN/4];
};

void sha256_reset(struct sha256 *sha);

static inline struct sha256 *sha256_init(void)
{
	struct sha256 *sha;

	sha = malloc(sizeof(struct sha256));
	if (sha)
		sha256_reset(sha);
	return sha;
}

static inline void sha256_exit(struct sha256 *sha)
{
	if (sha)
		free(sha);
}

void sha256(struct sha256 *hd, const unsigned char *buf, unsigned long len);

void sha256_file(struct sha256 *hd, FILE *fin);

static inline void sha256_to_str(unsigned char str[SHA_DGST_LEN],
		const unsigned char *buf, unsigned long len)
{
	struct sha256 sha;
	int i;

	sha256_reset(&sha);
	sha256(&sha, buf, len);
	for (i = 0; i < 8; i++)
		sha.H[i] = swap32(sha.H[i]);
	memcpy(str, sha.H, SHA_DGST_LEN);
}

#endif  /* SHA256_DSCAO__ */
