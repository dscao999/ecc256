#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "sha256_dgst.h"

int main(int argc, char *argv[])
{
	FILE *fin;
	unsigned char *buf;
	unsigned long mlen, lenrem, flen;
	struct stat filest;
	int nb, flag, i;
	unsigned int dgst[8];
	struct sha256_handle *hd;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s file\n", argv[0]);
		return 4;
	}

	if (stat(argv[1], &filest) == -1) {
		fprintf(stderr, "Bad file: %s->%s\n", argv[1], strerror(errno));
		return 100;
	}
	flen = filest.st_size;
	lenrem = flen;

	fin = fopen(argv[1], "rb");
	buf = malloc(SHA_BLOCK_LEN);
	hd = sha256_init();

	flag = SHA_START;
	mlen = 0;
	while (lenrem > 0) {
		nb = fread(buf, 1, SHA_BLOCK_LEN, fin);
		if (nb == 0) {
			fprintf(stderr, "File read error: %s\n", argv[1]);
			break;
		}
		mlen += nb;
		if (nb < SHA_BLOCK_LEN)
			flag |= SHA_END;
		sha256_block(hd, buf, dgst, mlen, flag);
		if (flag & SHA_END)
			break;
		flag = 0;
		lenrem -= nb;
	}
	if (!(flag & SHA_END))
		sha256_block(hd, buf, dgst, mlen, SHA_END);
	printf("SHA256(%s)= ", argv[1]);
	for (i = 0; i < 8; i++)
		printf("%08x", dgst[i]);
	printf("\n");
	free(buf);

	if (flen < 102400) {
		fseek(fin, 0, SEEK_SET);
		buf = malloc(flen);
		nb = fread(buf, 1, flen, fin);
		sha256(hd, buf, flen, dgst);
		fclose(fin);
		printf("SHA256(%s)= ", argv[1]);
		for (i = 0; i < 8; i++)
			printf("%08x", dgst[i]);
		printf("\n");

		free(buf);
	}
	sha256_exit(hd);

	return 0;
}
