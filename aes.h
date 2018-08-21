#ifndef AES_DSCAO__
#define AES_DSCAO__

void key_expan(unsigned char keys[176]);
void aes(const unsigned char ibytes[16], unsigned char obytes[16],
		const unsigned char w[176]);
void unaes(const unsigned char ibytes[16], unsigned char obytes[16],
		const unsigned char w[176]);

#endif /* AES_DSCAO__ */
