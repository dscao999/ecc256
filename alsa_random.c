#include <stdio.h>
#include "alsa_random.h"

#define SAMPLE_LEN	176400   /* one second of background noise, 44.1k * 2 channel * 16 bit */

struct alsa_param * alsa_init(int sec)
{
	int snderr, len;
	unsigned int srate;
	struct alsa_param *alsa;


	if (sec < 0)
		return NULL;
	else if (sec == 0)
		len = SAMPLE_LEN;
	else
		len = sec * 176400;
	alsa = malloc(sizeof(struct alsa_param));
	if (!alsa) {
		fprintf(stderr, "Out of Memory!\n");
		return NULL;
	}
	alsa->sha = sha256_init();
	if (alsa->sha == NULL) {
		fprintf(stderr, "Out of Memory!\n");
		goto err_5;
	}

	alsa->pcm_name = strdup("hw:0,0");
	if (!alsa->pcm_name) {
		fprintf(stderr, "Out of Memory!\n");
		goto err_10;
	}
	snderr = snd_pcm_hw_params_malloc(&alsa->hwparams);
	if (snderr < 0) {
		fprintf(stderr, "Cannot allocate parameter space: %s\n",
				snd_strerror(snderr));
		goto err_20;
	}
	alsa->stmtyp = SND_PCM_STREAM_CAPTURE;
	snderr = snd_pcm_open(&alsa->pcm_handle, alsa->pcm_name, alsa->stmtyp, 0);
	if (snderr < 0) {
		fprintf(stderr, "Error opening PCM device %s->%s\n",
				alsa->pcm_name, snd_strerror(snderr));
		goto err_30;
	}
	snderr = snd_pcm_hw_params_any(alsa->pcm_handle, alsa->hwparams);
	if (snderr < 0) {
		fprintf(stderr, "Can not configure this PCM device: %s\n", \
				snd_strerror(snderr));
		goto err_40;
	}
	snderr = snd_pcm_hw_params_set_access(alsa->pcm_handle, alsa->hwparams,
				SND_PCM_ACCESS_RW_INTERLEAVED);
	if (snderr < 0) {
		fprintf(stderr, "Cannot set access type: %s\n",
				snd_strerror(snderr));
		goto err_40;
	}
	snderr = snd_pcm_hw_params_set_format(alsa->pcm_handle, alsa->hwparams,
				SND_PCM_FORMAT_S16_LE);
	if (snderr < 0) {
		fprintf(stderr, "Cannot set sample format: %s\n",
				snd_strerror(snderr));
		goto err_40;
	}
	srate = SAMPLE_HZ;
	snderr = snd_pcm_hw_params_set_rate_near(alsa->pcm_handle, alsa->hwparams,
				&srate, 0);
	if (snderr < 0) {
		fprintf(stderr, "Cannot set sample rate: %s\n",
				snd_strerror(snderr));
		goto err_40;
	}
	snderr = snd_pcm_hw_params_set_channels(alsa->pcm_handle, alsa->hwparams,
				2);
	if (snderr < 0) {
		fprintf(stderr, "Cannot set channel number: %s\n",
				snd_strerror(snderr));
		goto err_40;
	}
	snderr = snd_pcm_hw_params(alsa->pcm_handle, alsa->hwparams);
	if (snderr < 0) {
		fprintf(stderr, "Cannot set parameters: %s\n",
				snd_strerror(snderr));
		goto err_40;
	}

	snderr = snd_pcm_prepare(alsa->pcm_handle);
	if (snderr < 0) {
		fprintf(stderr, "Prepare failed: %s\n", snd_strerror(snderr));
		goto err_40;
	}
	alsa->buf = malloc(len);
	if (!alsa->buf) {
		fprintf(stderr, "Out of Memory!\n");
		goto err_40;
	}
	alsa->buflen = len;
	alsa->paused = 0;
	snderr = snd_pcm_pause(alsa->pcm_handle, 1);
	if (snderr == 0)
		alsa->paused = 1;

	snd_pcm_hw_params_free(alsa->hwparams);
	return alsa;

err_40:
	snd_pcm_close(alsa->pcm_handle);
err_30:
	snd_pcm_hw_params_free(alsa->hwparams);
err_20:
	free(alsa->pcm_name);
err_10:
	sha256_exit(alsa->sha);
err_5:
	free(alsa);
	return NULL;
}

int alsa_random(struct alsa_param *alsa, unsigned int dgst[8])
{
	int snderr, retv;

	if (alsa->paused) {
		snderr = snd_pcm_pause(alsa->pcm_handle, 0);
		if (snderr)
			return snderr;
		alsa->paused = 0;
	}
	retv = 0;
	snderr = snd_pcm_readi(alsa->pcm_handle, alsa->buf, alsa->buflen/4);
	if (snderr != alsa->buflen/4) {
		retv = snderr;
		fprintf(stderr, "Warning! Audio read failed: %s\n",
			snd_strerror(snderr));
	}
	snderr = snd_pcm_pause(alsa->pcm_handle, 1);
	if (snderr == 0)
		alsa->paused = 1;
	sha256(alsa->sha, (unsigned char *)alsa->buf, alsa->buflen);
	memcpy(dgst, alsa->sha->H, 32);
	return retv;
}
