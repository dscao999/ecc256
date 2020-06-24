#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <gmp.h>
#include <stdlib.h>
#include "base64.h"

int rand32bytes(unsigned char rndbuf[32], int strong);

int main(int argc, char *argv[])
{
	int retv = 0, count;
	char strbuf[64], *bytes;
	unsigned int dgst[8], bkdgst[8];
	int strong = 0, i, len;
	size_t numw;
	mpz_t mbig;

	if (argc > 1)
		strong = atoi(argv[1]);

	if (strong < 0) {
		mpz_init(mbig);
		printf("Please Input a big integer: ");
		mpz_inp_str(mbig, stdin, 10);
		mpz_out_str(stdout, 10, mbig);
		printf("\n");
		numw = 0;
		mpz_export(dgst, &numw,	1, 4, 0, 0, mbig);
		for (i = 0; i < numw; i++)
			printf(" %08X ", dgst[i]);
		printf("\n");
		bytes = (char *)bkdgst;
		len = bin2str_b64(bytes, 32, (const unsigned char *)dgst, 4*numw);
		if (len < 0) {
			printf("B64 convert to string failed: %d\n", len);
			return 1;
		}
		bytes[len] = 0;
		printf("B64 String: %s\n", bytes);

		len = str2bin_b64((unsigned char *)dgst, 32, bytes);
		printf("str2bin len: %d\n", len);
		bytes = (char *)dgst;
		for (i = 0; i < len; i++)
			printf(" %02hhX ", bytes[i]);
		return 0;
	}
	count = 0;
	do {
		rand32bytes((unsigned char *)dgst, strong);
		bin2str_b64(strbuf, 64, (const unsigned char *)dgst, 32);
		printf("%s\n", strbuf);
		str2bin_b64((unsigned char *)bkdgst, 32, strbuf);
		for (i = 0; i < 8; i++)
			if (dgst[i] != bkdgst[i])
				printf("Failed!\n");
	} while (++count < 10);

	return retv;
}
