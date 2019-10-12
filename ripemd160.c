#include <assert.h>
#include <string.h>
#include "ripemd160.h"

static inline unsigned int
ripe_f(int j, unsigned int x, unsigned int y, unsigned int z)
{
	switch(j/16) {
	case 0:
		return x ^ y ^ z;
	case 1:
		return (x & y) | ((~x) & z);
	case 2:
		return (x | (~y)) ^ z;
	case 3:
		return (x & z) | (y & (~z));
	case 4:
		return x ^ (y | (~z));
	}
	assert(0);
}

static const unsigned int K[2][5] = {
	{0x00000000, 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xA953FD4E},
	{0x50A28BE6, 0x5C4DD124, 0x6D703EF3, 0x7A6D76E9, 0x00000000}
};
static inline unsigned int ripe_K(int t, int j)
{
	assert(t == 0 || t == 1);
	assert(j >= 0 && j < 80);
	return K[t][j/16];
}

static const int r_idx[2][80] = {
	{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
		7, 4, 13, 1, 10, 6, 15, 3, 12, 0, 9, 5, 2, 14, 11, 8,
		3, 10, 14, 4, 9, 15, 8, 1, 2, 7, 0, 6, 13, 11, 5, 12,
		1, 9, 11, 10, 0, 8, 12, 4, 13, 3, 7, 15, 14, 5, 6, 2,
		4, 0, 5, 9, 7, 12, 2, 10, 14, 1, 3, 8, 11, 6, 15, 13
	},
	{
		5, 14, 7, 0, 9, 2, 11, 4, 13, 6, 15, 8, 1, 10, 3, 12,
		6, 11, 3, 7, 0, 13, 5, 10, 14, 15, 8, 12, 4, 9, 1, 2,
		15, 5, 1, 3, 7, 14, 6, 9, 11, 8, 12, 2, 10, 0, 4, 13,
		8, 6, 4, 1, 3, 11, 15, 0, 5, 12, 2, 13, 9, 7, 10, 14,
		12, 15, 10, 4, 1, 5, 8, 7, 6, 2, 13, 14, 0, 3, 9, 11
	}
};
static inline unsigned int ripe_r(int t, int j, const unsigned int X[16])
{
	assert(t == 0 || t == 1);
	assert(j >= 0 && j < 80);
	return X[r_idx[t][j]];
}

static const int rol_bits[2][80] = {
	{
		11, 14, 15, 12, 5, 8, 7, 9, 11, 13, 14, 15, 6, 7, 9, 8,
		7, 6, 8, 13, 11, 9, 7, 15, 7, 12, 15, 9, 11, 7, 13, 12,
		11, 13, 6, 7, 14, 9, 13, 15, 14, 8, 13, 6, 5, 12, 7, 5,
		11, 12, 14, 15, 14, 15, 9, 8, 9, 14, 5, 6, 8, 6, 5, 12,
		9, 15, 5, 11, 6, 8, 13, 12, 5, 12, 13, 14, 11, 8, 5, 6
	},
	{
		8, 9, 9, 11, 13, 15, 15, 5, 7, 7, 8, 11, 14, 14, 12, 6,
		9, 13, 15, 7, 12, 8, 9, 11, 7, 7, 12, 7, 6, 15, 13, 11,
		9, 7, 15, 11, 8, 6, 6, 14, 12, 13, 5, 14, 13, 13, 7, 5,
		15, 5, 8, 11, 14, 14, 6, 14, 6, 9, 12, 9, 12, 5, 15, 8,
		8, 5, 12, 9, 12, 5, 14, 6, 8, 13, 6, 5, 15, 13, 11, 11
	}
};
static inline unsigned int ripe_rol(int t, int j, unsigned int X)
{
	int nobits;

	assert(t == 0 || t == 1);
	assert(j >= 0 && j < 80);
	nobits = rol_bits[t][j];
	return (X << nobits) | (X >> (32 - nobits));
}

static int ripe_padlen(unsigned char padded[64], unsigned long len,
		const unsigned char mesg[], int first)
{
	int rem;
	union {
		unsigned long len;
		unsigned char fill[8];
	} trailer;

	memset(padded, 0, 64);
	if (first) {
		rem = len & 63;
		if (rem)
			memcpy(padded, mesg, rem);
		padded[rem] = 0x80;
		if (rem + 9 > 64)
			return 0;
	}

	trailer.len = len * 8;
	memcpy(padded+56, trailer.fill, 8);
	return 1;
}

static void ripe_block(struct ripemd160 *ripe, const unsigned int H[16])
{
	unsigned int A, B, C, D, E, T;
	unsigned int At, Bt, Ct, Dt, Et, Tt;
	int j;

	A = ripe->H[0];
	B = ripe->H[1];
	C = ripe->H[2];
	D = ripe->H[3];
	E = ripe->H[4];
	At= ripe->H[0];
	Bt= ripe->H[1];
	Ct= ripe->H[2];
	Dt= ripe->H[3];
	Et= ripe->H[4];

	for (j = 0; j < 80; j++) {
		T = A + ripe_f(j, B, C, D) + ripe_r(0, j, H) + ripe_K(0, j);
		T = ripe_rol(0, j, T) + E;
		A = E;
		E = D;
		D = (C << 10) | (C >> 22);
		C = B;
		B = T;

		Tt = At + ripe_f(79-j, Bt, Ct, Dt) + ripe_r(1, j, H) + ripe_K(1, j);
		Tt = ripe_rol(1, j, Tt) + Et;
		At = Et;
		Et = Dt;
		Dt = (Ct << 10) | (Ct >> 22);
		Ct = Bt;
		Bt = Tt;
	}

	T = ripe->H[1] + C + Dt;
	ripe->H[1] = ripe->H[2] + D + Et;
	ripe->H[2] = ripe->H[3] + E + At;
	ripe->H[3] = ripe->H[4] + A + Bt;
	ripe->H[4] = ripe->H[0] + B + Ct;
	ripe->H[0] = T;
}

void ripemd160_dgst(struct ripemd160 *ripe, const unsigned char *msg, int len)
{
	unsigned char M[64];
	unsigned int *H;
	int pos = 0, done;

	H = (unsigned int *)M;
	while ((pos + 64) <= len) {
		memcpy(M, msg+pos, 64);
		pos += 64;
		ripe_block(ripe, H);
	}
	done = ripe_padlen(M, len, msg+pos, 1);
	ripe_block(ripe, H);
	if (!done) {
		ripe_padlen(M, len, NULL, 0);
		ripe_block(ripe, H);
	}
}

void ripemd160_fdgst(struct ripemd160 *ripe, FILE *fin)
{
	unsigned long len = 0;
	union {
		unsigned int M[16];
		char str[64];
	} buf;
	unsigned int M[16];
	int nbytes, done;

	nbytes = fread(buf.str, 1, 64, fin);
	while (nbytes == 64) {
		ripe_block(ripe, (unsigned int *)buf.M);
		len += nbytes;
		nbytes = fread(buf.str, 1, 64, fin);
	}
	if (nbytes == 0 && ferror(fin)) {
		fprintf(stderr, "Read file error!\n");
		return;
	}
	len += nbytes;
	done = ripe_padlen((unsigned char *)M, len,
			(unsigned char *)buf.str, 1);
	ripe_block(ripe, M);
	if (!done) {
		ripe_padlen((unsigned char *)M, len, NULL, 0);
		ripe_block(ripe, M);
	}
}
