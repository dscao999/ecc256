#include <string.h>
#include "md5.h"

static const wd8 padding[64] = {0x80};

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

void md5_init(struct md5_ctx *ctx)
{
	ctx->blen = 0;
	ctx->state[0] = 0x67452301;
	ctx->state[1] = 0xefcdab89;
	ctx->state[2] = 0x98badcfe;
	ctx->state[3] = 0x10325476;
}

static inline wd32 F(wd32 X, wd32 Y, wd32 Z)
{
	return (X & Y) | (~X & Z);
}
static inline wd32 G(wd32 X, wd32 Y, wd32 Z)
{
	return (X & Z) | (Y & ~Z);
}
static inline wd32 H(wd32 X, wd32 Y, wd32 Z)
{
	return X ^ Y ^ Z;
}
static inline wd32 I(wd32 X, wd32 Y, wd32 Z)
{
	return Y ^ ( X | ~Z);
}
static inline wd32 Rotate_Left(wd32 x, wd32 n)
{
	return (x << n) | (x >> (32-n));
}

static inline wd32 FF(wd32 a, wd32 b, wd32 c, wd32 d,
		wd32 x, wd32 s, wd32 ac)
{
	a += F(b, c, d) + x + ac;
	a = Rotate_Left(a, s);
	a += b;
	return a;
}
static inline wd32 GG(wd32 a, wd32 b, wd32 c, wd32 d,
		wd32 x, wd32 s, wd32 ac)
{
	a += G(b, c, d) + x + ac;
	a = Rotate_Left(a, s);
	a += b;
	return a;
}
static inline wd32 HH(wd32 a, wd32 b, wd32 c, wd32 d,
		wd32 x, wd32 s, wd32 ac)
{
	a += H(b, c, d) + x + ac;
	a = Rotate_Left(a, s);
	a += b;
	return a;
}
static inline wd32 II(wd32 a, wd32 b, wd32 c, wd32 d,
		wd32 x, wd32 s, wd32 ac)
{
	a += I(b, c, d) + x + ac;
	a = Rotate_Left(a, s);
	a += b;
	return a;
}

static void md5_transform(wd32 state[4], wd8 block[64])
{
	wd32 a, b, c, d, *x;

	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];

	/* valid only on little endian memory word */
	x = (wd32 *)block;

	a = FF(a, b, c, d, x[ 0], S11, 0xd76aa478);
	d = FF(d, a, b, c, x[ 1], S12, 0xe8c7b756);
	c = FF(c, d, a, b, x[ 2], S13, 0x242070db);
	b = FF(b, c, d, a, x[ 3], S14, 0xc1bdceee);

	a = FF(a, b, c, d, x[ 4], S11, 0xf57c0faf);
	d = FF(d, a, b, c, x[ 5], S12, 0x4787c62a);
	c = FF(c, d, a, b, x[ 6], S13, 0xa8304613);
	b = FF(b, c, d, a, x[ 7], S14, 0xfd469501);

	a = FF(a, b, c, d, x[ 8], S11, 0x698098d8);
	d = FF(d, a, b, c, x[ 9], S12, 0x8b44f7af);
	c = FF(c, d, a, b, x[10], S13, 0xffff5bb1);
	b = FF(b, c, d, a, x[11], S14, 0x895cd7be);

	a = FF(a, b, c, d, x[12], S11, 0x6b901122);
	d = FF(d, a, b, c, x[13], S12, 0xfd987193);
	c = FF(c, d, a, b, x[14], S13, 0xa679438e);
	b = FF(b, c, d, a, x[15], S14, 0x49b40821);

	a = GG(a, b, c, d, x[ 1], S21, 0xf61e2562);
	d = GG(d, a, b, c, x[ 6], S22, 0xc040b340);
	c = GG(c, d, a, b, x[11], S23, 0x265e5a51);
	b = GG(b, c, d, a, x[ 0], S24, 0xe9b6c7aa);

	a = GG(a, b, c, d, x[ 5], S21, 0xd62f105d);
	d = GG(d, a, b, c, x[10], S22,  0x2441453);
	c = GG(c, d, a, b, x[15], S23, 0xd8a1e681);
	b = GG(b, c, d, a, x[ 4], S24, 0xe7d3fbc8);

	a = GG(a, b, c, d, x[ 9], S21, 0x21e1cde6);
	d = GG(d, a, b, c, x[14], S22, 0xc33707d6);
	c = GG(c, d, a, b, x[ 3], S23, 0xf4d50d87);
	b = GG(b, c, d, a, x[ 8], S24, 0x455a14ed);

	a = GG(a, b, c, d, x[13], S21, 0xa9e3e905);
	d = GG(d, a, b, c, x[ 2], S22, 0xfcefa3f8);
	c = GG(c, d, a, b, x[ 7], S23, 0x676f02d9);
	b = GG(b, c, d, a, x[12], S24, 0x8d2a4c8a);

	a = HH(a, b, c, d, x[ 5], S31, 0xfffa3942);
	d = HH(d, a, b, c, x[ 8], S32, 0x8771f681);
	c = HH(c, d, a, b, x[11], S33, 0x6d9d6122);
	b = HH(b, c, d, a, x[14], S34, 0xfde5380c);

	a = HH(a, b, c, d, x[ 1], S31, 0xa4beea44);
	d = HH(d, a, b, c, x[ 4], S32, 0x4bdecfa9);
	c = HH(c, d, a, b, x[ 7], S33, 0xf6bb4b60);
	b = HH(b, c, d, a, x[10], S34, 0xbebfbc70);

	a = HH(a, b, c, d, x[13], S31, 0x289b7ec6);
	d = HH(d, a, b, c, x[ 0], S32, 0xeaa127fa);
	c = HH(c, d, a, b, x[ 3], S33, 0xd4ef3085);
	b = HH(b, c, d, a, x[ 6], S34,  0x4881d05);

	a = HH(a, b, c, d, x[ 9], S31, 0xd9d4d039);
	d = HH(d, a, b, c, x[12], S32, 0xe6db99e5);
	c = HH(c, d, a, b, x[15], S33, 0x1fa27cf8);
	b = HH(b, c, d, a, x[ 2], S34, 0xc4ac5665);

	a = II(a, b, c, d, x[ 0], S41, 0xf4292244);
	d = II(d, a, b, c, x[ 7], S42, 0x432aff97);
	c = II(c, d, a, b, x[14], S43, 0xab9423a7);
	b = II(b, c, d, a, x[ 5], S44, 0xfc93a039);

	a = II(a, b, c, d, x[12], S41, 0x655b59c3);
	d = II(d, a, b, c, x[ 3], S42, 0x8f0ccc92);
	c = II(c, d, a, b, x[10], S43, 0xffeff47d);
	b = II(b, c, d, a, x[ 1], S44, 0x85845dd1);

	a = II(a, b, c, d, x[ 8], S41, 0x6fa87e4f);
	d = II(d, a, b, c, x[15], S42, 0xfe2ce6e0);
	c = II(c, d, a, b, x[ 6], S43, 0xa3014314);
	b = II(b, c, d, a, x[13], S44, 0x4e0811a1);

	a = II(a, b, c, d, x[ 4], S41, 0xf7537e82);
	d = II(d, a, b, c, x[11], S42, 0xbd3af235);
	c = II(c, d, a, b, x[ 2], S43, 0x2ad7d2bb);
	b = II(b, c, d, a, x[ 9], S44, 0xeb86d391);

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
}

void md5_update(struct md5_ctx *ctx, const wd8 *msg, wd32 len)
{
	int bufpos, msgpos, cplen;

	msgpos = 0;
	bufpos = (ctx->blen >> 3) & 0x3f;
	ctx->blen += (len << 3);
	cplen = 64 - bufpos;
	if (msgpos + cplen <= len) {
		memcpy(ctx->buf+bufpos, msg+msgpos, cplen);
		md5_transform(ctx->state, ctx->buf);
		msgpos += cplen;
		bufpos = 0;
	}
	while (msgpos + 64 <= len) {
		memcpy(ctx->buf, msg+msgpos, 64);
		md5_transform(ctx->state, ctx->buf);
		msgpos += 64;
	}
	cplen = len - msgpos;
	memcpy(ctx->buf+bufpos, msg+msgpos, cplen);
}

void md5_exit(wd8 digest[16], struct md5_ctx *ctx)
{
	int bufpos, padlen;
	wd8 lenbits[8];

	memcpy(lenbits, (const char *)&ctx->blen, 8);
	bufpos = (ctx->blen >> 3) & 0x3f;
	padlen = (bufpos < 56)? (56 - bufpos) : (120 - bufpos);
	md5_update(ctx, padding, padlen);
	md5_update(ctx, lenbits, 8);
	memcpy(digest, ctx->state, 16);
}
