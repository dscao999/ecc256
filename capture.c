#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <assert.h>
#include <alsa/asoundlib.h>
#include "digest.h"

struct alsa_param {
	snd_pcm_t *pcm_handle;
	snd_pcm_stream_t stmtyp;
	snd_pcm_hw_params_t *hwparams;
	char *pcm_name;
};

#define SAMPLE_LEN	4096

int main(int argc, char *argv[])
{
	int retv = 0, snderr, numlen, idx, plen;
	struct alsa_param alsa;
	char *buf;
	unsigned char *byte;
	unsigned int srate, dgst[8];
	int blklen, param_done, i, opt;
	struct sha256_buf *shabuf;
	static struct option lopt[] = {
		{
			.name = "block",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'b'
		},
		{}
	};
	extern int opterr, optopt, optind;
	extern char *optarg;

	numlen = 0;
	blklen = 0;
	opterr = 0;
	param_done = 0;
	do {
		opt = getopt_long(argc, argv, ":b:", lopt, NULL);
		switch (opt) {
		case -1:
			param_done = 1;
			break;
		case '?':
			fprintf(stderr, "Unknown option: %c\n", optopt);
			break;
		case ':':
			fprintf(stderr, "Missing arguments for %c\n", optopt);
			break;
		case 'b':
			blklen = atoi(optarg);
			break;
		default:
			assert(0);
		}
	} while (!param_done);
	if (optind < argc)
		numlen = atoi(argv[optind]);
	if (numlen <= 0)
		numlen = 4;
	if (numlen > 32)
		numlen = 32;
	if (blklen == 0)
		blklen = SAMPLE_LEN;
	blklen = ((blklen >> 2) << 2);

	alsa.pcm_name = strdup("hw:0,0");
	if (!alsa.pcm_name) {
		fprintf(stderr, "Out of Memory!\n");
		return 10000;
	}
	alsa.stmtyp = SND_PCM_STREAM_CAPTURE;
	snderr = snd_pcm_open(&alsa.pcm_handle, alsa.pcm_name, alsa.stmtyp, 0);
	if (snderr < 0) {
		fprintf(stderr, "Error opening PCM device %s->%s\n",
				alsa.pcm_name, snd_strerror(snderr));
		retv = 100;
		goto exit_10;
	}
	snderr = snd_pcm_hw_params_malloc(&alsa.hwparams);
	if (snderr < 0) {
		fprintf(stderr, "Cannot allocate parameter space: %s\n",
				snd_strerror(snderr));
		retv = 10004;
		goto exit_20;
	}
	snderr = snd_pcm_hw_params_any(alsa.pcm_handle, alsa.hwparams);
	if (snderr < 0) {
		fprintf(stderr, "Can not configure this PCM device: %s\n", \
				snd_strerror(snderr));
		retv = 104;
		goto exit_30;
	}
	snderr = snd_pcm_hw_params_set_access(alsa.pcm_handle, alsa.hwparams,
				SND_PCM_ACCESS_RW_INTERLEAVED);
	if (snderr < 0) {
		fprintf(stderr, "Cannot set access type: %s\n",
				snd_strerror(snderr));
		retv = 108;
		goto exit_30;
	}
	snderr = snd_pcm_hw_params_set_format(alsa.pcm_handle, alsa.hwparams,
				SND_PCM_FORMAT_S16_LE);
	if (snderr < 0) {
		fprintf(stderr, "Cannot set sample format: %s\n",
				snd_strerror(snderr));
		retv = 112;
		goto exit_30;
	}
	srate = 44100;
	snderr = snd_pcm_hw_params_set_rate_near(alsa.pcm_handle, alsa.hwparams,
				&srate, 0);
	if (snderr < 0) {
		fprintf(stderr, "Cannot set sample rate: %s\n",
				snd_strerror(snderr));
		retv = 116;
		goto exit_30;
	}
	snderr = snd_pcm_hw_params_set_channels(alsa.pcm_handle, alsa.hwparams,
				2);
	if (snderr < 0) {
		fprintf(stderr, "Cannot set channel number: %s\n",
				snd_strerror(snderr));
		retv = 120;
		goto exit_30;
	}
	snderr = snd_pcm_hw_params(alsa.pcm_handle, alsa.hwparams);
	if (snderr < 0) {
		fprintf(stderr, "Cannot set parameters: %s\n",
				snd_strerror(snderr));
		retv = 124;
		goto exit_30;
	}

	buf = malloc(blklen);
	if (!buf) {
		fprintf(stderr, "Out of Memory!\n");
		retv = 10000;
		goto exit_30;
	}

	snderr = snd_pcm_prepare(alsa.pcm_handle);
	if (snderr < 0) {
		fprintf(stderr, "Prepare failed: %s\n", snd_strerror(snderr));
		retv = 128;
		goto exit_40;
	}

	snderr = snd_pcm_readi(alsa.pcm_handle, buf, blklen/4);
	if (snderr != blklen/4) {
		fprintf(stderr, "Audio read failed: %s\n", snd_strerror(snderr));
		retv = 132;
		goto exit_40;
	}

	shabuf = sha256_init();
	sha256(shabuf, (unsigned char *)buf, blklen, dgst);
	sha256_exit(shabuf);
	byte = (unsigned char *)buf;
	for (plen = 0; plen < 8; plen++) {
		printf("%08X", dgst[plen]);
		*byte++ = dgst[plen] >> 24;
		*byte++ = (dgst[plen] >> 16) & 0x0ff;
		*byte++ = (dgst[plen] >> 8) & 0x0ff;
		*byte++ = (dgst[plen]) & 0x0ff;
	}
	printf("\n");
	byte = (unsigned char *)buf;
	if (numlen < 32) {
		idx = buf[31] & 0x1f;
		for (i = 0; i < numlen; i++) {
			printf("%02X", (unsigned int)byte[idx]);
			idx = (idx + 1) & 0x1f;
		}
		printf("\n");
	}

exit_40:
	free(buf);
exit_30:
	snd_pcm_hw_params_free(alsa.hwparams);
exit_20:
	snd_pcm_close(alsa.pcm_handle);
exit_10:
	free(alsa.pcm_name);
	return retv;
}
