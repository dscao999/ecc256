#ifndef SHA256_DSCAO__
#define SHA256_DSCAO__

#define SHA_START  0x02
#define SHA_END 0x01

#define SHA_BLOCK_LEN	64

void sha256_block(unsigned char buf[64], unsigned int H[8], unsigned long len,
		int flag);

void sha256(unsigned char *buf, unsigned long len, unsigned H[8]);

#endif  /* SHA256_DSCAO__ */
