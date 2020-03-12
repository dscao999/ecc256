#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
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

static int bignum_insert(unsigned int bignums[], int num, int v, int bitpos)
{
	int wpos, bit_inw;
	unsigned int mw;

	wpos = num - (bitpos >> 5) - 1;
	if (wpos < 0)
		return 1;
	bit_inw = bitpos & 0x1f;
	mw = bignums[wpos];
	mw |= (v << bit_inw);
	bignums[wpos] = mw;
	if (bit_inw + B64_BITS > 31) {
		if (wpos == 0 && (v >> (32 - bit_inw)) != 0)
			return 1;
		mw = bignums[wpos-1];
		mw |= (v >> (32 - bit_inw));
		bignums[wpos-1] = mw;
	}
	return 0;
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
		ovflow = bignum_insert(bignums, num, digit, idx*B64_BITS);
		idx++;
	}

	return ovflow;
}

int str2bin_b64(unsigned char *binbytes, int num, const char *str)
{
	int bpos, bitpos, bbit, padded, shnxt;
	const char *pchr;
	char nchr;
	unsigned short nv, digit;

	memset(binbytes, 0, num);

	bpos = 0;
	padded = 0;
	bitpos = 0;
	bbit = 5;
	nv = 0;
	shnxt = 0;
	pchr = str;
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
		else if (nchr == '=') {
			padded = 1;
			break;
		} else
			break;

		shnxt = 0;
		bpos = bitpos >> 3;
		if (bpos == num)
			return -1;
		bbit =  7 - (bitpos & 7);
		if (bbit > 5)
			binbytes[bpos] |= (digit << (bbit - 5));
		else if (bbit < 5) {
			shnxt = 1;
			binbytes[bpos] |= (digit >> (5 - bbit));
			nv = (digit << (3 + bbit)) & 0x0ff;
			if (bpos == num - 1) {
				if (nv != 0)
					return -1;
			} else {
				binbytes[bpos+1] |= nv;
			}
		} else
			binbytes[bpos] |= digit;
		bitpos += B64_BITS;
	}
	if (bbit != 5 && (shnxt != 1 || nv != 0 || padded == 0))
		return -2;
	return bpos + 1;
}

int bin2str_b64(char *strbuf, int len, const unsigned char *binbytes, int num)
{
	int numbits, i;
	char *p64;
	unsigned char mc;
	int bpos, bbit, padded, rsb;
	unsigned short tmpc, mask;

	numbits = num << 3;

	padded = 0;
	p64 = strbuf;
	for (i = 0; i < numbits; i += B64_BITS) {
		bpos = (i >> 3);
		bbit = i & 7;
		rsb = 8 - (bbit + B64_BITS);
		tmpc = binbytes[bpos];
		mask = 0x0ff;
		if (rsb < 0) {
			if (bpos == num - 1) {
				padded = 1;
				mc = 0;
			} else
				mc = binbytes[bpos+1];
			mask = 0x0ffff;
			tmpc = (tmpc << 8) | mc;
			rsb += 8;
		}
		tmpc = ((tmpc << bbit) & mask) >> (bbit + rsb);
//		tmpc = tmpc >> rsb;
		assert(tmpc < 64);
		if (p64 < strbuf +  len)
			*p64 = BASE64_CHAR[tmpc];
		p64++;
	}
	if (padded) {
		if (p64 < strbuf + len)
			*p64 = '=';
		p64++;
	}
	if (p64 < strbuf + len)
		*p64 = 0;
	else
		*(strbuf+len-1) = 0;
	return p64 - strbuf;
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
//	strrev(buf);

	free(bignums);
	return idx;
}
