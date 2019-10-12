#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "ripemd160.h"
#include "base64.h"

int main(int argc, char *argv[])
{
	const char *msg;
	struct ripemd160 *ripe;
	int i, optfin = 0, opt, fname;
	FILE *fin;
	char buf[128];
	unsigned int dgst[5];
	extern int opterr, optopt, optind;

	opterr = 0;
	fname = 0;
	do {
		opt = getopt(argc, argv, ":f");
		switch(opt) {
		case '?':
			fprintf(stderr, "Unknown option: %c\n", (char)optopt);
			break;
		case 'f':
			fname = 1;
			break;
		case ':':
			fprintf(stderr, "Missing argument for %c\n", (char)opt);
			break;
		case -1:
			optfin = 1;
			break;
		default:
			assert(0);
		}
	} while (optfin != 1);
	if (optind < argc)
		msg = argv[optind];
	else
		msg = "";

	ripe = ripemd160_init();

	if (fname) {
		fin = fopen(msg, "rb");
		if (!fin) {
			fprintf(stderr, "Cannot open file: %s\n", msg);
			return 100;
		}
		ripemd160_fdgst(ripe, fin);
		fclose(fin);
	} else
		ripemd160_dgst(ripe, (const unsigned char *)msg, strlen(msg));
	for (i = 0; i < 5; i++) {
		printf("%02x", ripe->H[i] & 0x0ff);
		printf("%02x", (ripe->H[i] >> 8) & 0x0ff);
		printf("%02x", (ripe->H[i] >> 16) & 0x0ff);
		printf("%02x", (ripe->H[i] >> 24) & 0x0ff);
	}
	printf("\n");

	bignum2str_b64(buf, 128, ripe->H, 5);
	printf("%s\n", buf);
	ripemd160_exit(ripe);

	str2bignum_b64(dgst, 5, buf);
	for (i = 0; i < 5; i++) {
		printf("%02x", dgst[i] & 0x0ff);
		printf("%02x", (dgst[i] >> 8) & 0x0ff);
		printf("%02x", (dgst[i] >> 16) & 0x0ff);
		printf("%02x", (dgst[i] >> 24) & 0x0ff);
	}
	printf("\n");

	return 0;
}
