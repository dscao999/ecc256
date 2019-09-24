#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <assert.h>
#include "alsa_random.h"

int main(int argc, char *argv[])
{
	int retv = 0, keylen, idx, plen, count;
	struct alsa_param *alsa;
	unsigned char buf[32], *byte;
	unsigned int dgst[8];
	int sec, param_done, i, opt;
	static struct option lopt[] = {
		{
			.name = "sec",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'b'
		},
		{
			.name = "length",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'l'
		},
		{}
	};
	extern int opterr, optopt, optind;
	extern char *optarg;

	keylen = 0;
	sec = 0;
	opterr = 0;
	param_done = 0;
	do {
		opt = getopt_long(argc, argv, ":s:l:", lopt, NULL);
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
			sec = atoi(optarg);
			break;
		case 'l':
			keylen = atoi(optarg);
			break;
		default:
			assert(0);
		}
	} while (!param_done);
	if (keylen <= 0)
		keylen = 4;
	if (keylen > 32)
		keylen = 32;

	alsa = alsa_init(sec);
	if (!alsa)
		return 10000;

	count = 0;
	do {
		if (alsa_random(alsa, dgst) != 0) {
			fprintf(stderr, "Failed to get an random number!\n");
			break;
		}
		byte = buf;
		for (plen = 0; plen < 8; plen++) {
			printf("%08X", dgst[plen]);
			*byte++ = dgst[plen] >> 24;
			*byte++ = (dgst[plen] >> 16) & 0x0ff;
			*byte++ = (dgst[plen] >> 8) & 0x0ff;
			*byte++ = (dgst[plen]) & 0x0ff;
		}
		printf("\n");
		byte = (unsigned char *)buf;
		if (keylen < 32) {
			idx = buf[31] & 0x1f;
			for (i = 0; i < keylen; i++) {
				printf("%02X", (unsigned int)byte[idx]);
				idx = (idx + 1) & 0x1f;
			}
			printf("\n");
		}
	} while (++count < 10);

	alsa_exit(alsa);
	return retv;
}
