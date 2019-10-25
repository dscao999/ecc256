#ifndef ALSA_RANDOM_DSCAO__
#define ALSA_RANDOM_DSCAO__

#include <alsa/asoundlib.h>
#include "sha256.h"

#define SAMPLE_HZ 44100

struct alsa_param {
	struct sha256 *sha;
	snd_pcm_t *pcm_handle;
	snd_pcm_stream_t stmtyp;
	snd_pcm_hw_params_t *hwparams;
	char *pcm_name;
	char *buf;
	int buflen, paused;
};

struct alsa_param * alsa_init(const char *sdname, int sec);

static inline void alsa_exit(struct alsa_param *alsa)
{
	free(alsa->buf);
	snd_pcm_close(alsa->pcm_handle);
	sha256_exit(alsa->sha);
	free(alsa);
}

int alsa_random(struct alsa_param *alsa, unsigned int dgst[8]);

#endif /* ALSA_RANDOM_DSCAO__ */
