#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include "alsa_random.h"
#include "base64.h"

int main(int argc, char *argv[])
{
	int retv = 0, count;
	struct alsa_param *alsa;
	char strbuf[64];
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

	alsa = alsa_init(sdname, sec);
	if (!alsa)
		return 10000;

	count = 0;
	do {
		if (alsa_random(alsa, dgst) != 0) {
			fprintf(stderr, "Failed to get an random number!\n");
			break;
		}
		bignum2str_b64(strbuf, 64, dgst, 8);
		printf("%s\n", strbuf);
		str2bignum_b64(bkdgst, 8, strbuf);
		for (i = 0; i < 8; i++)
			if (dgst[i] != bkdgst[i])
				printf("Failed!\n");
	} while (++count < 10);

	alsa_exit(alsa);
	return retv;
}
