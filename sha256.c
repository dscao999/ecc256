#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "digest.h"

int main(int argc, char *argv[])
{
	FILE *fin;
	unsigned char *buf;
	unsigned long mlen, lenrem;
	struct stat filest;
	int nb, flag, i;
	unsigned int dgst[8];

	if (argc < 2) {
		fprintf(stderr, "Usage: %s file\n", argv[0]);
		return 4;
	}

	if (stat(argv[1], &filest) == -1) {
		fprintf(stderr, "Bad file: %s->%s\n", argv[1], strerror(errno));
		return 100;
	}
	lenrem = filest.st_size;

	fin = fopen(argv[1], "rb");
	buf = malloc(SHA_BLOCK_LEN);

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
		sha256(buf, dgst, mlen, flag);
		if (flag & SHA_END)
			break;
		flag = 0;
		lenrem -= nb;
	}
	if (!(flag & SHA_END))
		sha256(buf, dgst, mlen, SHA_END);
	fclose(fin);

	printf("SHA256:");
	for (i = 0; i < 8; i++)
		printf("%08x", dgst[i]);
	printf("\n");
	free(buf);
}
