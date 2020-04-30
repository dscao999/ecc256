#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include "alsarec.h"
#include "base64.h"

int main(int argc, char *argv[])
{
	int retv = 0, count, buflen;
	char strbuf[64];
	unsigned char *buf;
	unsigned int dgst[8], bkdgst[8];
	int sec = 0, i;
	const char *sdname = NULL;

	if (argc > 1)
		sec = atoi(argv[1]);
	if (sec == 0)
		sec = 5;
	if (argc > 2)
		sdname = argv[2];
	if (sdname == NULL)
		sdname = "hw:0,0";

	alsa_init(NULL);
	buflen = alsa_reclen(sec);
	buf = malloc(buflen);

	count = 0;
	do {
		if (alsa_record(sec, (unsigned char *)buf, buflen) < 0) {
			fprintf(stderr, "Failed to get an random number!\n");
			break;
		}
		alsa_random(dgst, buf, buflen);
		bin2str_b64(strbuf, 64, (const unsigned char *)dgst, 32);
		printf("%s\n", strbuf);
		str2bin_b64((unsigned char *)bkdgst, 32, strbuf);
		for (i = 0; i < 8; i++)
			if (dgst[i] != bkdgst[i])
				printf("Failed!\n");
	} while (++count < 10);

	return retv;
}
