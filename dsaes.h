#ifndef AES_DSCAO__
#define AES_DSCAO__
#include <stdlib.h>
#include "miscs.h"

#define AES128_BLOCK_LEN	16

struct aeskey {
	unsigned char w[176];
};

__declspec(dllexport) void __cdecl
aes_reset(struct aeskey *w, const unsigned char *pass);

struct aeskey * aes_init(const unsigned char *key);
static inline void aes_exit(struct aeskey *w)
{
	free(w);
}

void aes_block(const struct aeskey *w,
		const unsigned char *ibytes, unsigned char *obytes);
void unaes_block(const struct aeskey *w,
		const unsigned char *ibytes, unsigned char *obytes);

__declspec(dllexport) int __cdecl
dsaes(const struct aeskey *w,
		const unsigned char *buf, unsigned char *obuf, int len);

__declspec(dllexport) int __cdecl
un_dsaes(const struct aeskey *w,
		const unsigned char *buf, unsigned char *obuf, int len);

#endif /* AES_DSCAO__ */
