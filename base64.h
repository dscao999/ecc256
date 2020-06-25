#ifndef BASE64_DSCAO__
#define BASE64_DSCAO__

int bin2str_b64(char *buf, int len, const unsigned char *bigones, int num);

int str2bin_b64(unsigned char bigones[], int num, const char *buf);

#endif /* BASE64_DSCAO__ */
