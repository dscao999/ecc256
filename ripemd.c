#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include "ripemd160.h"
#include "base64.h"

int main(int argc, char *argv[])
{
	char *msgbuf, *fname;
	struct ripemd160 *ripe;
	int i;
	FILE *fin;
	struct stat fst;
	size_t flen;

	if (argc < 2)
		fprintf(stderr, "Usage: %s file\n", argv[0]);
	fname = argv[1];
	if (stat(fname, &fst) == -1) {
		fprintf(stderr, "Invalid file %s: %d\n", fname, errno);
		return 0;
	}
	flen = fst.st_size;

	ripe = ripemd160_init();

	fin = fopen(fname, "rb");
	if (!fin) {
		fprintf(stderr, "Cannot open file: %s\n", fname);
		return 100;
	}
	ripemd160_fdgst(ripe, fin);
	for (i = 0; i < 5; i++) {
		printf("%02x", ripe->H[i] & 0x0ff);
		printf("%02x", (ripe->H[i] >> 8) & 0x0ff);
		printf("%02x", (ripe->H[i] >> 16) & 0x0ff);
		printf("%02x", (ripe->H[i] >> 24) & 0x0ff);
	}
	printf("\n");

	rewind(fin);
	if (flen < 1048576) {
		ripemd160_reset(ripe);
		msgbuf = malloc(flen);
		fread(msgbuf, 1, flen, fin);
		ripemd160_dgst(ripe, (const unsigned char *)msgbuf, flen);
		for (i = 0; i < 5; i++) {
			printf("%02x", ripe->H[i] & 0x0ff);
			printf("%02x", (ripe->H[i] >> 8) & 0x0ff);
			printf("%02x", (ripe->H[i] >> 16) & 0x0ff);
			printf("%02x", (ripe->H[i] >> 24) & 0x0ff);
		}
		printf("\n");
	}
	fclose(fin);

	return 0;
}
