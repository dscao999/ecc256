#ifndef ALSA_RANDOM_DSCAO__
#define ALSA_RANDOM_DSCAO__

#include <alsa/asoundlib.h>

#define SAMPLE_HZ 44100

struct alsa_param {
	snd_pcm_t *pcm_handle;
	snd_pcm_stream_t stmtyp;
	snd_pcm_hw_params_t *hwparams;
	char *pcm_name;
	unsigned char *buf;
	int buflen, paused;
};

struct alsa_param * alsa_init(const char *sdname, int sec);

static inline void alsa_exit(struct alsa_param *alsa)
{
	free(alsa->buf);
	snd_pcm_close(alsa->pcm_handle);
	free(alsa->pcm_name);
	free(alsa);
}

int alsa_record(struct alsa_param *alsa);

void alsa_random(unsigned int dgst[8], const unsigned char *buf, int len);

#endif /* ALSA_RANDOM_DSCAO__ */
