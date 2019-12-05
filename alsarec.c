#include <stdio.h>
#include "alsarec.h"
#include "loglog.h"
#include "sha256.h"

#define SAMPLE_LEN	176400   /* one second of background noise, 44.1k * 2 channel * 16 bit */

struct alsa_param * alsa_init(const char *sdname, int sec)
{
	int snderr, len;
	unsigned int srate;
	struct alsa_param *alsa;


	if (sec < 0)
		return NULL;
	else if (sec == 0)
		len = SAMPLE_LEN;
	else
		len = sec * SAMPLE_LEN;
	alsa = malloc(sizeof(struct alsa_param));
	if (!alsa)
		return NULL;

	alsa->pcm_name = strdup(sdname);
	if (!alsa->pcm_name)
		goto err_10;
	snderr = snd_pcm_hw_params_malloc(&alsa->hwparams);
	if (snderr < 0)
		goto err_20;
	alsa->stmtyp = SND_PCM_STREAM_CAPTURE;
	snderr = snd_pcm_open(&alsa->pcm_handle, alsa->pcm_name, alsa->stmtyp, 0);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Error opening PCM device %s->%s\n",
				alsa->pcm_name, snd_strerror(snderr));
		goto err_30;
	}
	snderr = snd_pcm_hw_params_any(alsa->pcm_handle, alsa->hwparams);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Can not configure this PCM device: %s\n", \
				snd_strerror(snderr));
		goto err_40;
	}
	snderr = snd_pcm_hw_params_set_access(alsa->pcm_handle, alsa->hwparams,
				SND_PCM_ACCESS_RW_INTERLEAVED);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Cannot set access type: %s\n",
				snd_strerror(snderr));
		goto err_40;
	}
	snderr = snd_pcm_hw_params_set_format(alsa->pcm_handle, alsa->hwparams,
				SND_PCM_FORMAT_S16_LE);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Cannot set sample format: %s\n",
				snd_strerror(snderr));
		goto err_40;
	}
	srate = SAMPLE_HZ;
	snderr = snd_pcm_hw_params_set_rate_near(alsa->pcm_handle, alsa->hwparams,
				&srate, 0);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Cannot set sample rate: %s\n",
				snd_strerror(snderr));
		goto err_40;
	}
	snderr = snd_pcm_hw_params_set_channels(alsa->pcm_handle, alsa->hwparams,
				2);
	if (snderr < 0)
		snderr = snd_pcm_hw_params_set_channels(alsa->pcm_handle,
				alsa->hwparams, 1);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Cannot set channel number: %s\n",
				snd_strerror(snderr));
		goto err_40;
	}
	snderr = snd_pcm_hw_params(alsa->pcm_handle, alsa->hwparams);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Cannot set parameters: %s\n",
				snd_strerror(snderr));
		goto err_40;
	}

	snderr = snd_pcm_prepare(alsa->pcm_handle);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Prepare failed: %s\n", snd_strerror(snderr));
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
	free(alsa);
	return NULL;
}

int alsa_record(struct alsa_param *alsa)
{
	int snderr, retv;

	if (alsa->paused) {
		snderr = snd_pcm_pause(alsa->pcm_handle, 0);
		if (snderr) {
			logmsg(LOG_ERR, "Cannot Startup: %s\n",
					snd_strerror(snderr));
			return snderr;
		}
		alsa->paused = 0;
	}
	retv = 0;
	snderr = snd_pcm_readi(alsa->pcm_handle, alsa->buf, alsa->buflen/4);
	if (snderr != alsa->buflen/4) {
		logmsg(LOG_WARNING, "Warning! Audio read failed: %s\n",
			snd_strerror(snderr));
		logmsg(LOG_WARNING, "A Bad Random Number might be returned.\n");
		retv = snderr;
	}
	snderr = snd_pcm_pause(alsa->pcm_handle, 1);
	if (snderr == 0)
		alsa->paused = 1;
	return retv;
}

static struct sha256 sha;

void alsa_random(unsigned int dgst[8], const unsigned char *buf, int len)
{
	sha256_reset(&sha);
	sha256(&sha, (unsigned char *)buf, len);
	memcpy(dgst, sha.H, 32);
}
