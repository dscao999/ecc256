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
