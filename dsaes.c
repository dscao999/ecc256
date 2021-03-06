#include <string.h>
#include "dsaes.h"

static const unsigned char S[256] = {
0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};
static const unsigned char IS[256] = {
0x52,0x09,0x6A,0xD5,0x30,0x36,0xA5,0x38,0xBF,0x40,0xA3,0x9E,0x81,0xF3,0xD7,0xFB,
0x7C,0xE3,0x39,0x82,0x9B,0x2F,0xFF,0x87,0x34,0x8E,0x43,0x44,0xC4,0xDE,0xE9,0xCB,
0x54,0x7B,0x94,0x32,0xA6,0xC2,0x23,0x3D,0xEE,0x4C,0x95,0x0B,0x42,0xFA,0xC3,0x4E,
0x08,0x2E,0xA1,0x66,0x28,0xD9,0x24,0xB2,0x76,0x5B,0xA2,0x49,0x6D,0x8B,0xD1,0x25,
0x72,0xF8,0xF6,0x64,0x86,0x68,0x98,0x16,0xD4,0xA4,0x5C,0xCC,0x5D,0x65,0xB6,0x92,
0x6C,0x70,0x48,0x50,0xFD,0xED,0xB9,0xDA,0x5E,0x15,0x46,0x57,0xA7,0x8D,0x9D,0x84,
0x90,0xD8,0xAB,0x00,0x8C,0xBC,0xD3,0x0A,0xF7,0xE4,0x58,0x05,0xB8,0xB3,0x45,0x06,
0xD0,0x2C,0x1E,0x8F,0xCA,0x3F,0x0F,0x02,0xC1,0xAF,0xBD,0x03,0x01,0x13,0x8A,0x6B,
0x3A,0x91,0x11,0x41,0x4F,0x67,0xDC,0xEA,0x97,0xF2,0xCF,0xCE,0xF0,0xB4,0xE6,0x73,
0x96,0xAC,0x74,0x22,0xE7,0xAD,0x35,0x85,0xE2,0xF9,0x37,0xE8,0x1C,0x75,0xDF,0x6E,
0x47,0xF1,0x1A,0x71,0x1D,0x29,0xC5,0x89,0x6F,0xB7,0x62,0x0E,0xAA,0x18,0xBE,0x1B,
0xFC,0x56,0x3E,0x4B,0xC6,0xD2,0x79,0x20,0x9A,0xDB,0xC0,0xFE,0x78,0xCD,0x5A,0xF4,
0x1F,0xDD,0xA8,0x33,0x88,0x07,0xC7,0x31,0xB1,0x12,0x10,0x59,0x27,0x80,0xEC,0x5F,
0x60,0x51,0x7F,0xA9,0x19,0xB5,0x4A,0x0D,0x2D,0xE5,0x7A,0x9F,0x93,0xC9,0x9C,0xEF,
0xA0,0xE0,0x3B,0x4D,0xAE,0x2A,0xF5,0xB0,0xC8,0xEB,0xBB,0x3C,0x83,0x53,0x99,0x61,
0x17,0x2B,0x04,0x7E,0xBA,0x77,0xD6,0x26,0xE1,0x69,0x14,0x63,0x55,0x21,0x0C,0x7D
};
static const unsigned char RCON[24] = {
	0x8d, 0x01, 0x02, 0x04, 0x08, 0x10,
	0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c,
	0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e,
	0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a
};

static inline unsigned char sbox(unsigned char byte)
{
	return S[byte];
}
static inline unsigned char inv_sbox(unsigned char byte)
{
	return IS[byte];
}

static inline unsigned char dot(unsigned char a, unsigned char b)
{
	unsigned char res, fct, tmp;
	int i;

	fct = a;
	res = 0;
	for (i = 0; i < 8; i++) {
		if (b & 1)
			res ^= fct;
		b >>= 1;
		tmp = fct;
		fct <<= 1;
		if (tmp & 0x80)
			fct ^= 0x1b;
	}
	return res;
}

static inline void sbox_bytes(unsigned char b[], int len)
{
	int i;
	unsigned char *cb;

	for (cb = b, i = 0; i < len; i++, cb++)
		*cb = sbox(*cb);
}

static inline void inv_sbox_bytes(unsigned char b[], int len)
{
	int i;
	unsigned char *cb;

	for (cb = b, i = 0; i < len; i++, cb++)
		*cb = inv_sbox(*cb);
}

static inline void sbox_word(unsigned char b[4])
{
	sbox_bytes(b, 4);
}

static inline void rot_word(unsigned char b[4])
{
	unsigned char tb;

	tb = b[0];
	b[0] = b[1];
	b[1] = b[2];
	b[2] = b[3];
	b[3] = tb;
}

static inline unsigned char * sube(unsigned char *stw, int i, int j)
{
	return stw + j*4 + i;
}

static inline void s_row(unsigned char *st, int row)
{
	unsigned char ele;

	ele = *sube(st, row, 0);
	*sube(st, row, 0) = *sube(st, row, 1);
	*sube(st, row, 1) = *sube(st, row, 2);
	*sube(st, row, 2) = *sube(st, row, 3);
	*sube(st, row, 3) = ele;
}

static inline void invs_row(unsigned char *st, int row)
{
	unsigned char ele;

	ele = *sube(st, row, 3);
	*sube(st, row, 3) = *sube(st, row, 2);
	*sube(st, row, 2) = *sube(st, row, 1);
	*sube(st, row, 1) = *sube(st, row, 0);
	*sube(st, row, 0) = ele;
}

static void shift_rows(unsigned char *st)
{
	s_row(st, 1);
	s_row(st, 2);
	s_row(st, 2);
	s_row(st, 3);
	s_row(st, 3);
	s_row(st, 3);
}

static void inv_shift_rows(unsigned char *st)
{
	invs_row(st, 1);
	invs_row(st, 2);
	invs_row(st, 2);
	invs_row(st, 3);
	invs_row(st, 3);
	invs_row(st, 3);
}

static void dot_vec(unsigned char w[4], unsigned char s[4])
{
	s[0] = dot(2, w[0]) ^ dot(3, w[1]) ^ w[2] ^ w[3];
	s[1] = w[0] ^ dot(2, w[1]) ^ dot(3, w[2]) ^ w[3];
	s[2] = w[0] ^ w[1] ^ dot(2, w[2]) ^ dot(3, w[3]);
	s[3] = dot(3, w[0]) ^ w[1] ^ w[2] ^ dot(2, w[3]);
}

static void inv_dot_vec(unsigned char w[4], unsigned char s[4])
{
	s[0] = dot(0x0e, w[0]) ^ dot(0x0b, w[1]) ^ dot(0x0d, w[2]) ^ dot(0x09, w[3]);
	s[1] = dot(0x09, w[0]) ^ dot(0x0e, w[1]) ^ dot(0x0b, w[2]) ^ dot(0x0d, w[3]);
	s[2] = dot(0x0d, w[0]) ^ dot(0x09, w[1]) ^ dot(0x0e, w[2]) ^ dot(0x0b, w[3]);
	s[3] = dot(0x0b, w[0]) ^ dot(0x0d, w[1]) ^ dot(0x09, w[2]) ^ dot(0x0e, w[3]);
}

static inline void mix_columns(unsigned char *s)
{
	unsigned char w[AES128_BLOCK_LEN];

	memcpy(w, s, AES128_BLOCK_LEN);
	dot_vec(w, s);
	dot_vec(w+4, s+4);
	dot_vec(w+8, s+8);
	dot_vec(w+12, s+12);
}

static inline void inv_mix_columns(unsigned char *s)
{
	unsigned char w[AES128_BLOCK_LEN];

	memcpy(w, s, AES128_BLOCK_LEN);
	inv_dot_vec(w, s);
	inv_dot_vec(w+4, s+4);
	inv_dot_vec(w+8, s+8);
	inv_dot_vec(w+12, s+12);
}

static inline void add_round_key(unsigned char *stw,
		const unsigned char *w)
{
	int i;

	for (i = 0; i < AES128_BLOCK_LEN; i++)
		stw[i] ^= w[i];
}

void aes_block(const struct aeskey *w,
		const unsigned char *ibytes, unsigned char *obytes)
{
	int r;

	memcpy(obytes, ibytes, AES128_BLOCK_LEN);
	add_round_key(obytes, w->w);

	for (r = 1; r < 10; r++) {
		sbox_bytes(obytes, AES128_BLOCK_LEN);
		shift_rows(obytes);
		mix_columns(obytes);
		add_round_key(obytes, w->w+r*AES128_BLOCK_LEN);
	}
	sbox_bytes(obytes, AES128_BLOCK_LEN);
	shift_rows(obytes);
	add_round_key(obytes, w->w+160);
}

void unaes_block(const struct aeskey *w,
		const unsigned char *ibytes, unsigned char *obytes)
{
	int r;

	memcpy(obytes, ibytes, AES128_BLOCK_LEN);
	add_round_key(obytes, w->w+160);

	for (r = 9; r >= 1; r--) {
		inv_shift_rows(obytes);
		inv_sbox_bytes(obytes, AES128_BLOCK_LEN);
		add_round_key(obytes, w->w+r*AES128_BLOCK_LEN);
		inv_mix_columns(obytes);
	}
	inv_shift_rows(obytes);
	inv_sbox_bytes(obytes, AES128_BLOCK_LEN);
	add_round_key(obytes, w->w);
}

static void key_expan(unsigned char keys[176])
{
	int i, j, k;
	unsigned char ktmp[4];

	for (i = 4; i < 44; i++) {
		j = i * 4;
		ktmp[0] = keys[j-4];
		ktmp[1] = keys[j-3];
		ktmp[2] = keys[j-2];
		ktmp[3] = keys[j-1];
		if ((i & 3) == 0) {
			k = i >> 2;
			rot_word(ktmp);
			sbox_word(ktmp);
			ktmp[0] ^= RCON[k];
		}
		keys[j] = keys[j-16] ^ ktmp[0];
		keys[j+1] = keys[j-15] ^ ktmp[1];
		keys[j+2] = keys[j-14] ^ ktmp[2];
		keys[j+3] = keys[j-13] ^ ktmp[3];
	}
}

void aes_reset(struct aeskey *w, const unsigned char *pass)
{
	memcpy(w->w, pass, AES128_BLOCK_LEN);
	key_expan(w->w);
}

struct aeskey * aes_init(const unsigned char *key)
{
	struct aeskey *w;

	w = malloc(sizeof(struct aeskey));
	if (!w)
		return w;
	aes_reset(w, key);
	return w;
}

int dsaes(const struct aeskey *key, const unsigned char *ibuf,
		unsigned char *obuf, int len)
{
	const unsigned char *blkin;
	unsigned char *blkout;
	int pos;

	if ((len % AES128_BLOCK_LEN) != 0)
		return -1;

	blkin = ibuf;
	blkout = obuf;
	for (pos = 0; pos + AES128_BLOCK_LEN <= len; pos += AES128_BLOCK_LEN) {
		aes_block(key, blkin, blkout);
		blkin += AES128_BLOCK_LEN;
		blkout += AES128_BLOCK_LEN;
	}
	return 0;
}

int un_dsaes(const struct aeskey *key, const unsigned char *ibuf,
		unsigned char *obuf, int len)
{
	const unsigned char *blkin;
	unsigned char *blkout;
	int pos;

	if ((len % AES128_BLOCK_LEN) != 0)
		return -1;

	blkin = ibuf;
	blkout = obuf;
	for (pos = 0; pos + AES128_BLOCK_LEN <= len; pos += AES128_BLOCK_LEN) {
		unaes_block(key, blkin, blkout);
		blkin += AES128_BLOCK_LEN;
		blkout += AES128_BLOCK_LEN;
	}
	return 0;
}
