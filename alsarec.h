#ifndef ALSA_RANDOM_DSCAO__
#define ALSA_RANDOM_DSCAO__

#include <alsa/asoundlib.h>

int alsa_init(const char *sdname);
void alsa_exit(void);

int alsa_reclen(int sec);
int alsa_record(int sec, unsigned char *buf, int buflen);

void alsa_random(unsigned int dgst[8], const unsigned char *buf, int len);

int noise_random(unsigned int dgst[8], int sec);

#endif /* ALSA_RANDOM_DSCAO__ */
