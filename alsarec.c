#include <stdio.h>
#include "alsarec.h"
#include "loglog.h"
#include "sha256.h"

#define SAMPLE_LEN	176400   /* one second of background noise, 44.1k * 2 channel * 16 bit */

static char sdname[32];
static snd_pcm_hw_params_t *hwparams;

static void alsa_dev_search(void)
{
	snd_ctl_t *hdl;
	snd_pcm_info_t *pcminfo = NULL;
	snd_ctl_card_info_t *info = NULL;
	int snderr, card, stream, sddev;

	snd_ctl_card_info_malloc(&info);
	snd_pcm_info_malloc(&pcminfo);
	stream = SND_PCM_STREAM_CAPTURE;
	card = -1;
	while (snd_card_next(&card) >= 0 && card >= 0) {
		sprintf(sdname, "hw:%d", card);
		snderr = snd_ctl_open(&hdl, sdname, 0);
		if (snderr < 0) {
			logmsg(LOG_ERR, "control open (%d): %s\n", card,
					snd_strerror(snderr));
			continue;
		}
		if ((snderr = snd_ctl_card_info(hdl, info)) < 0) {
			logmsg(LOG_ERR, "control hardware info (%i): %s", card,
					snd_strerror(snderr));
			snd_ctl_close(hdl);
			continue;
		}
		sddev = -1;
		sdname[0] = 0;
		while (snd_ctl_pcm_next_device(hdl, &sddev) >= 0 && sddev >= 0) {
			snd_pcm_info_set_device(pcminfo, sddev);
			snd_pcm_info_set_subdevice(pcminfo, 0);
			snd_pcm_info_set_stream(pcminfo, stream);
			snderr = snd_ctl_pcm_info(hdl, pcminfo);
			if (snderr >= 0) {
				sprintf(sdname, "hw:%d,%d", card, sddev);
				break;
			}
			if (snderr != -ENOENT)
				logmsg(LOG_ERR, "control digital audio"
					       " info (%i): %s", card,
					       snd_strerror(snderr));
		}
		snd_ctl_close(hdl);
		if (sdname[0] != 0)
			break;
	}
	snd_pcm_info_free(pcminfo);
	snd_ctl_card_info_free(info);
	snd_config_update_free_global();
	if (sdname[0] == 0)
		logmsg(LOG_ERR, "No Sound Card!\n");
}

int alsa_init(const char *sdevname)
{
	snd_pcm_t *pcmhdl;
	int snderr, retv = 0;
	unsigned int noch, srate;

	if (!sdevname)
		alsa_dev_search();
	else
		strcpy(sdname, sdevname);

	snderr = snd_pcm_hw_params_malloc(&hwparams);
	if (snderr < 0) {
		logmsg(LOG_CRIT, nomem);
		return -100;
	}
	snderr = snd_pcm_open(&pcmhdl, sdname, SND_PCM_STREAM_CAPTURE, 0);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Error opening PCM device %s->%s\n",
				sdname, snd_strerror(snderr));
		goto err_30;
	}
	snderr = snd_pcm_hw_params_any(pcmhdl, hwparams);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Can not configure this PCM device: %s\n", \
				snd_strerror(snderr));
		goto err_40;
	}
	snderr = snd_pcm_hw_params_set_access(pcmhdl, hwparams,
			SND_PCM_ACCESS_RW_INTERLEAVED);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Cannot set access type: %s\n",
				snd_strerror(snderr));
		goto err_50;
	}
	snderr = snd_pcm_hw_params_set_format(pcmhdl, hwparams,
			SND_PCM_FORMAT_S16_LE);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Cannot set sample format: %s\n",
				snd_strerror(snderr));
		goto err_50;
	}
	snderr = snd_pcm_hw_params(pcmhdl, hwparams);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Cannot set parameters: %s\n",
				snd_strerror(snderr));
		goto err_50;
	}
	snderr = snd_pcm_hw_params_get_rate(hwparams, &srate, NULL);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Cannot set sample rate: %s\n",
				snd_strerror(snderr));
		goto err_50;
	}
	snderr = snd_pcm_hw_params_get_channels(hwparams, &noch);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Cannot get channel number: %s\n",
				snd_strerror(snderr));
		goto err_50;
	}
	snd_pcm_close(pcmhdl);

	return retv;

err_50:
	snd_pcm_hw_params_free(hwparams);
	hwparams = NULL;
err_40:
	snd_pcm_close(pcmhdl);
err_30:
	sdname[0] = 0;
	return retv;
}

void alsa_exit(void)
{
	snd_pcm_hw_params_free(hwparams);
}

int alsa_reclen(int secs)
{
	unsigned int noch, srate;
	int snderr, len = -1;

	if (hwparams == NULL) {
		logmsg(LOG_ERR, "Random Source not Initialized!\n");
		return len;
	}

	snderr = snd_pcm_hw_params_get_rate(hwparams, &srate, NULL);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Cannot set sample rate: %s\n",
				snd_strerror(snderr));
		goto err_40;
	}
	snderr = snd_pcm_hw_params_get_channels(hwparams, &noch);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Cannot get channel number: %s\n",
				snd_strerror(snderr));
		goto err_40;
	}
	len = sizeof(unsigned short) * noch * secs * srate;

err_40:
	return len;
}

int alsa_record(int sec, unsigned char *buf, int buflen)
{
	snd_pcm_t *pcmhdl;
	int snderr, retv = 0, frame_len;
	unsigned int noch;

	if (sdname[0] == 0) {
		logmsg(LOG_ERR, "No Random Source!]n");
		return -1;
	}
	snderr = snd_pcm_open(&pcmhdl, sdname, SND_PCM_STREAM_CAPTURE, 0);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Error opening PCM device %s->%s\n",
				sdname, snd_strerror(snderr));
		return -1;
	}
	snderr = snd_pcm_hw_params_get_channels(hwparams, &noch);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Cannot get channel number: %s\n",
				snd_strerror(snderr));
		goto err_10;
	}
	snderr = snd_pcm_hw_params(pcmhdl, hwparams);
	if (snderr < 0) {
		logmsg(LOG_ERR, "Cannot set parameters: %s\n",
				snd_strerror(snderr));
		retv = snderr;
		goto err_10;
	}
	snderr = snd_pcm_prepare (pcmhdl);
	if (snderr < 0) {
		logmsg(LOG_ERR, "cannot prepare audio interface for use (%s)\n",
				snd_strerror(snderr));
		retv = snderr;
		goto err_10;
	}

	frame_len = sizeof(unsigned short) * noch;
	snderr = snd_pcm_readi(pcmhdl, buf, buflen/frame_len);
	if (snderr != buflen/frame_len) {
		logmsg(LOG_WARNING, "Warning! Audio read failed: %s\n",
			snd_strerror(snderr));
		logmsg(LOG_WARNING, "A Bad Random Number might be returned.\n");
	}
	retv = snderr;

err_10:
	snd_pcm_close(pcmhdl);
	return retv;
}


void alsa_random(unsigned int dgst[8], const unsigned char *buf, int len)
{
	struct sha256 sha;

	sha256_reset(&sha);
	sha256(&sha, (unsigned char *)buf, len);
	memcpy(dgst, sha.H, 32);
}
