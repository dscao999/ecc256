#ifndef AES_DSCAO__
#define AES_DSCAO__

void aes_init(const unsigned char key[16]);
void aes_block(const unsigned char ibytes[16], unsigned char obytes[16]);
void unaes_block(const unsigned char ibytes[16], unsigned char obytes[16]);

void aes(const unsigned char *buf, unsigned char *obuf, int len);
void unaes(const unsigned char *buf, unsigned char *obuf, int len);

#endif /* AES_DSCAO__ */
