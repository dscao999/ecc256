#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "sha256.h"
#include "base64.h"

int main(int argc, char *argv[])
{
	FILE *fin;
	ulong64 flen;
	struct stat filest;
	int i, nb;
	struct sha256 *hd;
	char *buf, b64str[64];
	unsigned int M[8];

	if (argc < 2) {
		fprintf(stderr, "Usage: %s file\n", argv[0]);
		return 4;
	}

	if (stat(argv[1], &filest) == -1) {
		fprintf(stderr, "Bad file: %s->%s\n", argv[1], strerror(errno));
		return 100;
	}
	flen = filest.st_size;

	hd = sha256_init();

	fin = fopen(argv[1], "rb");
	sha256_file(hd, fin);

	for (i = 0; i < 8; i++)
		printf("%08x", hd->H[i]);
	printf("\n");
	bignum2str_b64(b64str, 64, hd->H, 8);
	str2bignum_b64(M, 8, b64str);
	if (memcmp(hd->H, M, 32) != 0)
		fprintf(stderr, "Bad base64 operation!\n");

	if (flen < 1048576) {
		sha256_reset(hd);
		fseek(fin, 0, SEEK_SET);
		buf = malloc(flen);
		nb = fread(buf, 1, flen, fin);
		assert(nb == flen);
		sha256(hd, (unsigned char *)buf, flen);
		for (i = 0; i < 8; i++)
			printf("%08x", hd->H[i]);
		printf("\n");

		free(buf);
	}
	fclose(fin);

	sha256_exit(hd);
	printf("%s\n", b64str);

	return 0;
}
