#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "sha256.h"

int main(int argc, char *argv[])
{
	FILE *fin;
	unsigned long flen;
	struct stat filest;
	int i, nb;
	struct sha256 *hd;
	char *buf;

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

	printf("SHA256(%s)= ", argv[1]);
	for (i = 0; i < 8; i++)
		printf("%08x", hd->H[i]);
	printf("\n");

	if (flen < 102400) {
		sha256_reset(hd);
		fseek(fin, 0, SEEK_SET);
		buf = malloc(flen);
		nb = fread(buf, 1, flen, fin);
		assert(nb == flen);
		sha256(hd, (unsigned char *)buf, flen);
		printf("SHA256(%s)= ", argv[1]);
		for (i = 0; i < 8; i++)
			printf("%08x", hd->H[i]);
		printf("\n");

		free(buf);
	}
	fclose(fin);

	sha256_exit(hd);

	return 0;
}
