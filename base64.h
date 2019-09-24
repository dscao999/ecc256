#ifndef BASE64_DSCAO__
#define BASE64_DSCAO__

int bignum2str_b64(char *buf, int len, const unsigned int bigones[], int num);

int str2bignum_b64(unsigned int bigones[], int num, const char *buf);

#endif /* BASE64_DSCAO__ */
