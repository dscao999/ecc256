#ifndef AES_DSCAO__
#define AES_DSCAO__
#include <stdlib.h>

#define AES128_BLOCK_LEN	16

struct aeskey {
	unsigned char w[176];
};

struct aeskey * aes_init(const unsigned char key[16]);
static inline void aes_exit(struct aeskey *w)
{
	free(w);
}

void aes_block(const struct aeskey *w,
		const unsigned char ibytes[16], unsigned char obytes[16]);
void unaes_block(const struct aeskey *w,
		const unsigned char ibytes[16], unsigned char obytes[16]);

int aes(const struct aeskey *w,
		const unsigned char *buf, unsigned char *obuf, int len);
int unaes(const struct aeskey *w,
		const unsigned char *buf, unsigned char *obuf, int len);

#endif /* AES_DSCAO__ */