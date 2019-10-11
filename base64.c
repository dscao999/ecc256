#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "base64.h"

#define B64_BITS	6
#define B64_MASK	0x3f

static const char BASE64_CHAR[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
	'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
	'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

static inline char *strrev(char *str)
{
	char *p1, *p2;

	if (!str || !*str)
		return str;
	for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}
	return str;
}

static int bignum_leadzero(unsigned int bignums[], int num)
{
	int i;

	for (i = 0; i < num && bignums[i] == 0; i++)
		;
	return i;
}

static unsigned int bignum_rshift(unsigned int bignums[], int num)
{
	int i, nzero;
	unsigned int retv = 0;

	nzero = bignum_leadzero(bignums, num);
	if (nzero == num)
		return retv;

	retv = bignums[num-1] & B64_MASK;
	for (i = num - 1; i > nzero; i--)
		bignums[i] = (bignums[i] >> B64_BITS) |
				((bignums[i-1] & B64_MASK) << (32 - B64_BITS));
	bignums[nzero] >>= B64_BITS;

	return retv;
}

static int bignum_lshift(unsigned int bignums[], int num, int v)
{
	int i, nzero, retv = 0, ovflow;

	nzero = bignum_leadzero(bignums, num);
	if (nzero == num) {
		bignums[num-1] = (v & B64_MASK);
		return retv;
	}

	ovflow = bignums[nzero] >> (32 - B64_BITS);
	if (nzero > 0)
		bignums[nzero-1] = ovflow;
	else
		retv = ovflow;
	for (i = nzero; i < num - 1; i++)
		bignums[i] = (bignums[i] << B64_BITS) |
				(bignums[i+1] >> (32 - B64_BITS));
	bignums[num-1] = (bignums[num-1] << B64_BITS) | (v & B64_MASK);

	return retv;
}

int str2bignum_b64(unsigned int bignums[], int num, const char *buf)
{
	int i, digit, idx, ovflow = 0;
	const char *pchr;
	char nchr;

	for (i = 0; i < num; i++)
		bignums[i] = 0;
	idx = 0;
	pchr = buf;
	while (*pchr != 0) {
		nchr = *pchr++;
		if (nchr >= 'A' && nchr <= 'Z')
			digit = nchr - 'A';
		else if (nchr >= 'a' && nchr <= 'z')
			digit = (nchr - 'a') + 26;
		else if (nchr >= '0' && nchr <= '9')
			digit = (nchr - '0') + 52;
		else if (nchr == '+')
			digit = 62;
		else if (nchr == '/')
			digit = 63;
		else
			return idx;
		idx++;
		ovflow = bignum_lshift(bignums, num, digit);
	}

	return ovflow;
}

int bignum2str_b64(char *buf, int len, const unsigned int bigones[], int num)
{
	int idx, i, rembits, nobits, buflen;
	char digit;
	unsigned int *bignums;

	bignums = malloc(sizeof(unsigned int)*num);
	if (!bignums) {
		fprintf(stderr, "Out of Memory!\n");
		exit(100);
	}
	memcpy(bignums, bigones, sizeof(unsigned int)*num);

	buflen = len - 1;
	nobits = (num * 32) / B64_BITS;
	rembits = (num * 32) % B64_BITS;
	for (idx = 0, i = 0; i < nobits; i++, idx++) {
		digit = BASE64_CHAR[bignum_rshift(bignums, num)];
		if (idx < buflen)
			buf[idx] = digit;
	}
	if (rembits) {
		digit = BASE64_CHAR[bignums[num-1] & B64_MASK];
		if (idx < buflen)
			buf[idx] = digit;
		idx++;
	}
	if (idx < len)
		buf[idx] = 0;
	else
		buf[len-1] = 0;
	strrev(buf);

	free(bignums);
	return idx;
}
