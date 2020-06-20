#include <portaudio.h>
#include <time.h>
#include <string.h>
#include "loglog.h"
#include "paudio_op.h"
#include "sha256.h"

struct uopt {
	volatile ulong64 frames;
	ulong64 limit;
	unsigned char *buf;
	const PaDeviceInfo *devinfo;
	PaStreamParameters iparam;
	int devidx;
	int ulen;
};
static struct uopt uop;

static int recv_data(const void *ibuf, void *obuf, ulong64 fcount,
		const PaStreamCallbackTimeInfo *tmInfo,
		PaStreamCallbackFlags statusFlags, void *dat)
{
	struct uopt *puop = dat;
	int pos, lenrem, lencur;

	if (puop->frames < puop->limit) {
		lenrem = (puop->limit - puop->frames) * puop->ulen;
		pos = puop->frames * puop->ulen;
		lencur = fcount * puop->ulen;
		memcpy(puop->buf+pos, ibuf, lencur > lenrem? lenrem : lencur);
		puop->frames += fcount;
	}
	return paContinue;
}

void paudio_exit(void)
{
	Pa_Terminate();
}

int paudio_init(void)
{
	PaError paerr;
	int numdevs, i, retv = 0;
	const PaDeviceInfo *devInfo;

	paerr = Pa_Initialize();
	if (unlikely(paerr != paNoError)) {
		logmsg(LOG_ERR, "Pa_Initialize return: %s\n",
				Pa_GetErrorText(paerr));
		return 1;
	}
	numdevs = Pa_GetDeviceCount();
	if (unlikely(numdevs < 0)) {
		logmsg(LOG_ERR, "Pa_GetDeviceCount return: %d\n", numdevs);
		retv = 2;
		goto exit_10;
	}
	uop.devidx = -1;
	for (i = 0; i < numdevs; i++) {
		devInfo = Pa_GetDeviceInfo(i);
		if (devInfo && devInfo->maxInputChannels > 0) {
			uop.devidx = i;
			uop.devinfo = devInfo;
			break;
		}
	}
	if (uop.devidx == -1) {
		logmsg(LOG_ERR, "No Audio Device has no input!\n");
		retv = 3;
		goto exit_10;
	}
	uop.iparam.device = uop.devidx;
	uop.iparam.channelCount = uop.devinfo->maxInputChannels;
	uop.iparam.sampleFormat = paInt16;
	uop.iparam.suggestedLatency = 0;
	uop.iparam.hostApiSpecificStreamInfo = NULL;

	uop.ulen = sizeof(unsigned short)*uop.devinfo->maxInputChannels;
	uop.frames = 0;
	uop.limit = 0;
	uop.buf = NULL;
	return 0;

exit_10:
	Pa_Terminate();
	return retv;
}

int paudio_reclen(int sec)
{
	return sec * uop.devinfo->defaultSampleRate * uop.ulen;
}

int paudio_rec(int sec, char *buf, int len)
{
	PaError paerr;
	int retv = 0;
	PaStream *istrm;
	struct timespec tm;

	uop.limit = sec * uop.devinfo->defaultSampleRate;
	if (uop.limit * uop.ulen > len) {
		logmsg(LOG_ERR, "paudio_rec buffer too short.\n");
		return -3;
	}
	uop.frames = 0;
	uop.buf = (unsigned char *)buf;
	paerr = Pa_OpenStream(&istrm, &uop.iparam, NULL,
			uop.devinfo->defaultSampleRate, 0, paNoFlag,
			recv_data, &uop);
	if (unlikely(paerr != paNoError)) {
		logmsg(LOG_ERR, "Pa_OpenStream return: %s\n",
				Pa_GetErrorText(paerr));
		return -4;
	}

	tm.tv_sec = 0;
	tm.tv_nsec = 100000000;
	paerr = Pa_StartStream(istrm);
	if (unlikely(paerr != paNoError)) {
		logmsg(LOG_ERR, "Pa_StartStream return: %s\n",
				Pa_GetErrorText(paerr));
		retv = -5;
		goto exit_10;
	}
	while (uop.frames < uop.limit)
		nanosleep(&tm, NULL);
	paerr = Pa_StopStream(istrm);
	if (unlikely(paerr != paNoError)) {
		logmsg(LOG_ERR, "Pa_StopStream return: %s\n", Pa_GetErrorText(paerr));
		retv = -6;
	}

exit_10:
	Pa_CloseStream(istrm);
	return retv;
}

static struct sha256 sha;

void paudio_random(unsigned int dgst[8], const unsigned char *buf, int len)
{
	sha256_reset(&sha);
	sha256(&sha, buf, len);
	memcpy(dgst, sha.H, 32);
}
