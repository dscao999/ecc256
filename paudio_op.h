#ifndef PAUDIO_OP_DSCAO__
#define PAUDIO_OP_DSCAO__
int paudio_init(void);
void paudio_exit(void);
int paudio_reclen(int sec);
int paudio_rec(int sec, char *buf, int len);
void paudio_random(unsigned int dgst[8], const unsigned char *buf, int len);
#endif  /* PAUDIO_OP_DSCAO__ */
