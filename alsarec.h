#ifndef ALSA_RANDOM_DSCAO__
#define ALSA_RANDOM_DSCAO__

#include <alsa/asoundlib.h>

int alsa_init(void);
void alsa_exit(void);

int alsa_reclen(int sec);
int alsa_record(int sec, char *buf, int buflen);

void alsa_random(unsigned int dgst[8], const unsigned char *buf, int len);

#endif /* ALSA_RANDOM_DSCAO__ */
