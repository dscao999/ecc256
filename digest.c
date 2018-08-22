/*
 *  SHA3-256 Message digest implementation 
 *  FIPS PUB 180-4 Secure Hash Standard
 *  Dashi Cao, caods1@lenovo.com, dscao999@hotmail.com
 */
#include <string.h>
#include "digest.h"

#define ShR(X, b) ((X) >> b)
#define RotR(X, b)  (((X) >> b) | ((X) << (32 - b)))
#define Ch(X, Y, Z) (((X) & (Y)) ^ (~(X) & (Z)))
#define Maj(X, Y, Z) (((X) & (Y)) ^ ((X) & (Z)) ^ ((Y) & (Z)))

#define BigSigma0(X) (RotR(X, 2) ^ RotR(X, 13) ^ RotR(X, 22))
#define BigSigma1(X) (RotR(X, 6) ^ RotR(X, 11) ^ RotR(X, 25))
#define Sigma0(X) (RotR(X, 7) ^ RotR(X, 18) ^ ShR(X, 3))
#define Sigma1(X) (RotR(X, 17) ^ RotR(X, 19) ^ ShR(X, 10))

const static unsigned int K[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
	0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
	0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
	0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
	0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
	0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

const static unsigned int H0[8] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

static const unsigned char pad[56] = { 0x80, 0 };

static void sha_padlen(unsigned char buf[8], unsigned long len)
{
	buf[0] = (len >> 56) & 0x0ff;
	buf[1] = (len >> 48) & 0x0ff;
	buf[2] = (len >> 40) & 0x0ff;
	buf[3] = (len >> 32) & 0x0ff;
	buf[4] = (len >> 24) & 0x0ff;
	buf[5] = (len >> 16) & 0x0ff;
	buf[6] = (len >> 8) & 0x0ff;
	buf[7] = len & 0x0ff;
}

static unsigned int sha_buf2word(const unsigned char buf[4])
{
	return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

static void sha_block(const unsigned char buf[64], unsigned int H[8])
{
	unsigned int W[64];
	int i;
	unsigned int T1, T2;
	unsigned int a, b, c, d, e, f, g, h;

	for (i = 0; i < 16; i++)
		W[i] = sha_buf2word(buf+i*4);

	for (i = 16; i < 64; i++)
		W[i] = Sigma1(W[i-2]) + W[i-7] + Sigma0(W[i-15]) + W[i-16];
	a = H[0];
	b = H[1];
	c = H[2];
	d = H[3];
	e = H[4];
	f = H[5];
	g = H[6];
	h = H[7];

	for (i = 0; i < 64; i++) {
		T1 = h + BigSigma1(e) + Ch(e, f, g) + K[i] + W[i];
		T2 = BigSigma0(a) + Maj(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + T1;
		d = c;
		c = b;
		b = a;
		a = T1 + T2;
	}
	H[0] += a;
	H[1] += b;
	H[2] += c;
	H[3] += d;
	H[4] += e;
	H[5] += f;
	H[6] += g;
	H[7] += h;
}

void sha256_block(struct sha256_handle *hd, const unsigned char buf[64],
		unsigned int H[8], unsigned long len, int flag)
{
	int padlen, modlen;
	unsigned char *pad_block;

	if (flag & SHA_START) {
		H[0] = H0[0];
		H[1] = H0[1];
		H[2] = H0[2];
		H[3] = H0[3];
		H[4] = H0[4];
		H[5] = H0[5];
		H[6] = H0[6];
		H[7] = H0[7];
	}
	if (flag & SHA_END) {
		pad_block = hd->pad_block;
		modlen = len % 64;
		memcpy(pad_block, buf, modlen);
		if (modlen < 56) {
			padlen = 56 - modlen;
			memcpy(pad_block+modlen, pad, padlen);
			sha_padlen(pad_block+56, len*8);
			sha_block(pad_block, H);
		} else {
			padlen = 64 - modlen;
			memcpy(pad_block+modlen, pad, padlen);
			sha_block(pad_block, H);
			memset(pad_block, 0, 56);
			sha_padlen(pad_block+56, len*8);
			sha_block(pad_block, H);
		}
	} else
		sha_block(buf, H);
}

void sha256(struct sha256_handle *hd, const unsigned char *buf, unsigned long len,
		unsigned H[8])
{
	const unsigned char *block;
	unsigned long curpos;
	int flag;

	flag = SHA_START;
	block = buf;
	curpos = 0;
	while (curpos + SHA_BLOCK_LEN <= len) {
		sha256_block(hd, block, H, len, flag);
		block += SHA_BLOCK_LEN;
		curpos += SHA_BLOCK_LEN;
		flag = 0;
	}
	flag |= SHA_END;
	sha256_block(hd, block, H, len, flag);
}
