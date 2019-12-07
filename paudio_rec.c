#include <stdio.h>
#include <portaudio.h>
#include <time.h>
#include <string.h>
#include "loglog.h"
#include "sha256.h"

struct uopt {
	volatile unsigned long frames;
	unsigned long limit;
	unsigned char *buf;
	const PaDeviceInfo *devusb;
};

static void save2file(struct uopt *op)
{
	FILE *of;

	of = fopen("/tmp/rnoise.dat", "wb");
	fwrite(op->buf, 1, op->frames*sizeof(unsigned short)*op->devusb->maxInputChannels, of);
	fclose(of);
}

static int recv_data(const void *ibuf, void *obuf, unsigned long fcount,
		const PaStreamCallbackTimeInfo *tmInfo,
		PaStreamCallbackFlags statusFlags, void *dat)
{
	struct uopt *uop = dat;
	int pos, unitlen, lenrem, lencur;

	unitlen = sizeof(unsigned short)*uop->devusb->maxInputChannels;
	if (uop->frames < uop->limit) {
		lenrem = (uop->limit - uop->frames) * unitlen;
		pos = uop->frames * unitlen;
		lencur = fcount * unitlen;
		memcpy(uop->buf+pos, ibuf, lencur > lenrem? lenrem : lencur);
		uop->frames += fcount;
	}
	return paContinue;
}

int main(int argc, char *argv[])
{
	PaError paerr;
	int numdevs, i, retv = 0, adusb = -1;
	const PaDeviceInfo *devInfo;
	PaStreamParameters iparam;
	PaStream *istrm;
	struct uopt uop;
	struct timespec tm;

	paerr = Pa_Initialize();
	if (unlikely(paerr != paNoError)) {
		logmsg(LOG_ERR, "Pa_Initialize return: %s\n", Pa_GetErrorText(paerr));
		return 1;
	}
	numdevs = Pa_GetDeviceCount();
	if (unlikely(numdevs < 0)) {
		logmsg(LOG_ERR, "Pa_GetDeviceCount return: %d\n", numdevs);
		retv = 2;
		goto exit_10;
	}
	for (i = 0; i < numdevs; i++) {
		devInfo = Pa_GetDeviceInfo(i);
		if (strstr(devInfo->name, "USB Audio Device") != NULL) {
			adusb = i;
			uop.devusb = devInfo;
		}
	}

	if (adusb == 0 || uop.devusb->maxInputChannels < 1) {
		logmsg(LOG_ERR, "No USB Audio Device has no input!\n");
		retv = 3;
		goto exit_10;
	}
	iparam.device = adusb;
	iparam.channelCount = uop.devusb->maxInputChannels;
	iparam.sampleFormat = paInt16;
	iparam.suggestedLatency = 0;
	iparam.hostApiSpecificStreamInfo = NULL;

	uop.frames = 0;
	uop.limit = uop.devusb->defaultSampleRate * 3;
	uop.buf = malloc(uop.limit*sizeof(unsigned short)*uop.devusb->maxInputChannels);
	if (!check_pointer(uop.buf, LOG_CRIT, nomem)) {
		retv = 100;
		goto exit_10;
	}

	paerr = Pa_OpenStream(&istrm, &iparam, NULL, uop.devusb->defaultSampleRate,
			0, paNoFlag, recv_data, &uop);
	if (unlikely(paerr != paNoError)) {
		logmsg(LOG_ERR, "Pa_OpenStream return: %s\n", Pa_GetErrorText(paerr));
		retv = 4;
		goto exit_20;
	}

	tm.tv_sec = 0;
	tm.tv_nsec = 100000000;
	paerr = Pa_StartStream(istrm);
	if (unlikely(paerr != paNoError)) {
		logmsg(LOG_ERR, "Pa_StartStream return: %s\n", Pa_GetErrorText(paerr));
		retv = 6;
		goto exit_30;
	}
	while (uop.frames < uop.limit)
		nanosleep(&tm, NULL);
	paerr = Pa_StopStream(istrm);
	if (unlikely(paerr != paNoError)) {
		logmsg(LOG_ERR, "Pa_StopStream return: %s\n", Pa_GetErrorText(paerr));
		retv = 6;
	}
	printf("Total frames got: %lu\n", uop.frames);
	save2file(&uop);

exit_30:
	Pa_CloseStream(istrm);
exit_20:
	free(uop.buf);
exit_10:
	Pa_Terminate();
	return retv;
}
