/*
 * Elliptic Curve Cryptography
 * Implementation of secp256k1, y exp 2 = x exp 3 + 7.
 * Dashi Cao, dscao999@hotmail.com, caods1@lenovo.com
 *
 */
#include <stdio.h>
#if defined(__linux__)
#include <gmp.h>
#elif defined(_WIN64)
#include "mpir.h"
#endif
#include <assert.h>
#include <errno.h>
#include <string.h>
#include "ecc_secp256k1.h"
#include "base64.h"
#include "dscrc.h"
#include "dsaes.h"
#include "ripemd160.h"
#include "loglog.h"
#include "sha256.h"

#define BITLEN	256

#define CBYTE const unsigned char
#define BYTE unsigned char

#include "ecc_G_data.h"

static const unsigned int EPM[] = {
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFE, 0xFFFFFC2F
};
static const unsigned int EPN[] = {
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE, 0xBAAEDCE6, 0xAF48A03B,
	0xBFD25E8C, 0xD0364141
};
static mpz_t epm, epn, sroot;
static const unsigned short b = 7;

#include "gmp_wrapper.h"

static inline int moduli_3_4(void)
{
	mpz_t r;
	int rm;

	mpz_init2(r, 256);
	rm = (int)mpz_mod_ui(r, epm, 4);
	mpz_clear(r);
	return rm == 3;
}

static void a_exp(mpz_t x, const mpz_t a, const mpz_t e)
{
	unsigned int w[ECCKEY_INT_LEN], cw;
	size_t rlen;
	mpz_t factor, sum, tmpnum;
	int i, j;

	mpz_init2(factor, 2*BITLEN);
	mpz_init2(sum, 2*BITLEN);
	mpz_init2(tmpnum, 2*BITLEN);
	mpz_set_ui(sum, 1);
	mpz_set(factor, a);
	mpz_export(w, &rlen, 1, 4, 0, 0, e);
	for (i = (int)(rlen-1); i >= 0; i--) {
		cw = w[i];
		for (j = 0; j < 32; j++) {
			if ((cw & 1) == 1)
				mpz_mul_mod(sum, sum, factor);
			cw >>= 1;
			mpz_mul_mod(tmpnum, factor, factor);
			mpz_set(factor, tmpnum);
		}
	}
	mpz_set(x, sum);
	mpz_clears(sum, factor, tmpnum, NULL);
}

static inline void a_sroot(mpz_t x, const mpz_t a)
{
	a_exp(x, a, sroot);
}

struct curve_point {
	mpz_t x;
	mpz_t y;
};

static struct curve_point G[256];

static inline void point_init(struct curve_point *P)
{
	mpz_init2(P->x, 2*BITLEN);
	mpz_init2(P->y, 2*BITLEN);
}

static inline void point_clear(struct curve_point *P)
{
	mpz_clear(P->x);
	mpz_clear(P->y);
}

static inline
void point_set(struct curve_point *P, const struct curve_point *Q)
{
	mpz_set(P->x, Q->x);
	mpz_set(P->y, Q->y);
}

static inline
void point_set_ui(struct curve_point *P, unsigned int x, unsigned int y)
{
	mpz_set_ui(P->x, x);
	mpz_set_ui(P->y, y);
}

static int is_on_curve(const struct curve_point *P)
{
	int retv;
	mpz_t x, y;

	mpz_init2(x, 2*BITLEN);
	mpz_init2(y, 2*BITLEN);
	mpz_mul_mod(y, P->y, P->y);
	mpz_mul_mod(x, P->x, P->x);
	mpz_mul_mod(x, x, P->x);
	mpz_add_ui_mod(x, x, b);

	retv = mpz_cmp(x, y) == 0;
	mpz_clears(x, y, NULL);

	return retv;
}

static inline void point_assign(struct curve_point *P,
			const unsigned int px[], const unsigned int py[])
{
	mpz_import(P->x, ECCKEY_INT_LEN, 1, 4, 0, 0, px);
	mpz_import(P->y, ECCKEY_INT_LEN, 1, 4, 0, 0, py);
	assert(is_on_curve(P));
}

static int point_line_lambd(mpz_t lambd, const struct curve_point *P,
		const struct curve_point *Q)
{
	mpz_t ztmp, wtmp;
	int nosig, retv;

	nosig = 1;
	mpz_init2(ztmp, 2*BITLEN);
	mpz_init2(wtmp, 2*BITLEN);
	mpz_sub_mod(ztmp, Q->y, P->y);
	mpz_sub_mod(wtmp, Q->x, P->x);
	if (mpz_cmp_ui(wtmp, 0) == 0)
		nosig = 0;
	else {
		retv = mpz_invert(wtmp, wtmp, epm);
		assert(retv != 0);
		mpz_mul_mod(lambd, ztmp, wtmp);
	}

	mpz_clears(ztmp, wtmp, NULL);
	return nosig;
}

static int point_tagent_lambd(mpz_t lambd, const struct curve_point *P)
{
	mpz_t ztmp;
	int retv, nosig;

	mpz_init2(ztmp, 512);

	nosig = 1;
	if (mpz_cmp_ui(P->y, 0) == 0)
		nosig = 0;
	else {
		mpz_mul_mod(lambd, P->x, P->x);
		mpz_mul_ui_mod(lambd, lambd, 3);
		mpz_mul_ui_mod(ztmp, P->y, 2);
		retv = mpz_invert(ztmp, ztmp, epm);
		assert(retv != 0);
		mpz_mul_mod(lambd, lambd, ztmp);
	}

	mpz_clear(ztmp);

	return nosig;
}

static inline int point_equal(const struct curve_point *P,
			const struct curve_point *Q)
{
	return mpz_cmp(P->x, Q->x) == 0 && mpz_cmp(P->y, Q->y) == 0;
}

static inline int point_zero(const struct curve_point *P)
{
	return mpz_cmp_ui(P->x, 0) == 0 && mpz_cmp_ui(P->y, 0) == 0;
}

static void point_add(struct curve_point *R, const struct curve_point *P,
			const struct curve_point *Q)
{
	mpz_t lambd, ztmp;
	struct curve_point tmpR;

	mpz_init2(lambd, 2*BITLEN);
	mpz_init2(ztmp, 2*BITLEN);
	point_init(&tmpR);

	if (point_zero(P)) {
		point_set(&tmpR, Q);
	} else if (point_zero(Q)) {
		point_set(&tmpR, P);
	} else if (point_equal(P, Q)) {
		if (point_tagent_lambd(lambd, P)) {
			mpz_mul_mod(ztmp, lambd, lambd);
			mpz_sub_mod(ztmp, ztmp, P->x);
			mpz_sub_mod(tmpR.x, ztmp, P->x);
			mpz_sub_mod(ztmp, P->x, tmpR.x);
			mpz_mul_mod(ztmp, ztmp, lambd);
			mpz_sub_mod(tmpR.y, ztmp, P->y);
		} else {
			point_set_ui(&tmpR, 0, 0);
		}
	} else {
		if (point_line_lambd(lambd, P, Q)) {
			mpz_mul_mod(ztmp, lambd, lambd);
			mpz_sub_mod(ztmp, ztmp, P->x);
			mpz_sub_mod(tmpR.x, ztmp, Q->x);
			mpz_sub_mod(ztmp, P->x, tmpR.x);
			mpz_mul_mod(ztmp, ztmp, lambd);
			mpz_sub_mod(tmpR.y, ztmp, P->y);
		} else {
			point_set_ui(&tmpR, 0, 0);
		}
	}
	point_set(R, &tmpR);
	mpz_clears(lambd, ztmp, NULL);
	point_clear(&tmpR);
}

static inline void point_double(struct curve_point *R,
			const struct curve_point *P)
{
	point_add(R, P, P);
}

static void point_x_num_G(struct curve_point *R, mpz_t num)
{
	unsigned int x[ECCKEY_INT_LEN], cnum;
	size_t count;
	int i, j, idx;

	point_set_ui(R, 0, 0);

	mpz_export(x, &count, 1, 4, 0, 0, num);
	assert(count <= ECCKEY_INT_LEN);
	idx = 0;
	for (i = (int)(count-1); i >= 0; i--) {
		cnum = x[i];
		for (j = 0; j < 32; j++) {
			if (cnum & 1)
				point_add(R, R, G+idx);
			cnum >>= 1;
			idx++;
		}
	}
}

static void point_x_num_nG(struct curve_point *R, const mpz_t num,
			const struct curve_point *H)
{
	struct curve_point S, tmpR;
	unsigned int x[ECCKEY_INT_LEN], cnum;
	size_t count;
	int i, j;

	point_init(&tmpR);
	point_set_ui(&tmpR, 0, 0);
	point_init(&S);
	point_set(&S, H);

	mpz_export(x, &count, 1, 4, 0, 0, num);
	assert(count <= ECCKEY_INT_LEN);
	for (i = (int)(count-1); i >= 0; i--) {
		cnum = x[i];
		for (j = 0; j < 32; j++) {
			if (cnum & 1)
				point_add(&tmpR, &tmpR, &S);
			cnum >>= 1;
			point_double(&S, &S);
		}
	}
	point_set(R, &tmpR);
	point_clear(&S);
	point_clear(&tmpR);
}

static int ecc_check(void)
{
	int retv = 0;
	struct curve_point P;
	unsigned int x[ECCKEY_INT_LEN], y[ECCKEY_INT_LEN];
	size_t count_x, count_y;

	point_init(&P);
	point_x_num_G(&P, epn);
	mpz_export(x, &count_x, 1, 4, 0, 0, P.x);
	mpz_export(y, &count_y, 1, 4, 0, 0, P.y);
	if (count_x != 0 || count_y != 0)
		retv = 1;
	
	point_clear(&P);
	return retv;
}

void ecc_init(void)
{
	int i;

	mpz_init2(epm, BITLEN);
	mpz_import(epm, ECCKEY_INT_LEN, 1, 4, 0, 0, EPM);
	mpz_init2(epn, BITLEN);
	mpz_import(epn, ECCKEY_INT_LEN, 1, 4, 0, 0, EPN);
	for (i = 0; i < 256; i++) {
		point_init(G+i);
		point_assign(G+i, (Gxy+i)->x, (Gxy+i)->y);
	}

	mpz_init2(sroot, BITLEN);
	mpz_add_ui(sroot, epm, 1);
	mpz_fdiv_q_2exp(sroot, sroot, 2);

	assert(mpz_probab_prime_p(epm, 64) != 0);
	assert(mpz_probab_prime_p(epn, 64) != 0);
	assert(moduli_3_4());
	assert(ecc_check() == 0);
}

void ecc_exit(void)
{
	int i;

	mpz_clear(epm);
	for (i = 0; i < 256; i++)
		point_clear(G+i);
	mpz_clear(epn);
	mpz_clear(sroot);
}

static void compute_public(struct ecc_key *ecckey, int flag)
{
	struct curve_point P;
	size_t count_x, count_y;
	mpz_t x, tmp;

	point_init(&P);
	mpz_init2(x, BITLEN);

	switch(flag) {
	case 0:
		mpz_import(x, ECCKEY_INT_LEN, 1, 4, 0, 0, ecckey->pr);
		point_x_num_G(&P, x);
		mpz_export(ecckey->px, &count_x, 1, 4, 0, 0, P.x);
		mpz_export(ecckey->py, &count_y, 1, 4, 0, 0, P.y);
		assert(count_x != 0 || count_y != 0);
		break;
	case 1:
	case 2:
		mpz_import(x, ECCKEY_INT_LEN, 1, 4, 0, 0, ecckey->px);
		mpz_init2(tmp, 2*BITLEN);
		mpz_mul_mod(tmp, x, x);
		mpz_mul_mod(tmp, tmp, x);
		mpz_add_ui_mod(tmp, tmp, b);
		a_sroot(x, tmp);
		mpz_export(ecckey->py, &count_y, 1, 4, 0, 0, x);
		if ((flag == 1 && (ecckey->py[ECCKEY_INT_LEN-1] & 1) == 0) ||
				(flag == 2 && 
				(ecckey->py[ECCKEY_INT_LEN-1] & 1) == 1)) {
			mpz_sub(x, epm, x);
			mpz_export(ecckey->py, &count_y, 1, 4, 0, 0, x);
		}
		mpz_clear(tmp);
		break;
	}

	mpz_clear(x);
	point_clear(&P);
}

void ecc_get_public_y(struct ecc_key *ekey, int flag)
{
	assert(flag == 1 || flag == 2);
	compute_public(ekey, flag);
}

void ecc_get_public(const unsigned char *skey, struct ecc_key *ekey)
{
	memcpy(ekey->pr, skey, ECCKEY_INT_LEN*4);
	compute_public(ekey, 0);
}

int ecc_genkey(struct ecc_key *ecckey)
{
	int retv;
	mpz_t x;

	mpz_init2(x, BITLEN);
	do {
		retv = rand32bytes((unsigned char *)ecckey->pr, 32, 1);
		if (retv != 32)
			return -1;
		mpz_import(x, ECCKEY_INT_LEN, 1, 4, 0, 0, ecckey->pr);
	} while (mpz_cmp(x, epn) >= 0 || mpz_cmp_ui(x, 0) == 0);
	compute_public(ecckey, 0);
	mpz_clear(x);

	return 0;
}

void ecc_writkey(const struct ecc_key *ecckey, unsigned char bt[48],
		const char *ps, int len)
{
	unsigned char buf[48];
	struct ripemd160 ripe;
	struct aeskey aes;
	int reclen;
	unsigned int *crc, ucrc;

	reclen = ECCKEY_INT_LEN + AES128_BLOCK_LEN/4;
	assert(reclen == 12);
	ripemd160_reset(&ripe);
	ripemd160_dgst(&ripe, (CBYTE *)ps, len);
	aes_reset(&aes, (CBYTE *)ripe.H);
	memcpy(buf, ecckey->pr, ECCKEY_INT_LEN*4);
	crc = (unsigned int *)(buf + (reclen - 1) * 4);
	ucrc = crc32(buf, (reclen - 1)*4);
	*crc = swap32(ucrc);
	dsaes(&aes, buf, bt, reclen*4);
}

int ecc_readkey(struct ecc_key *ecckey, const unsigned char bt[48],
		const char *ps, int len)
{
	struct ripemd160 ripe;
	struct aeskey aes;
	int reclen, retv = 0;

	reclen = ECCKEY_INT_LEN+AES128_BLOCK_LEN/4;
	assert(reclen == 12);
	ripemd160_reset(&ripe);
	ripemd160_dgst(&ripe, (CBYTE *)ps, len);
	aes_reset(&aes, (CBYTE *)ripe.H);
	un_dsaes(&aes, bt, (BYTE *)ecckey->pr, reclen*4);
	if (crc32((CBYTE *)ecckey->pr, reclen*4)) {
		logmsg(LOG_ERR, "Invalid passphrase.\n");
		retv = -1;
	} else
		compute_public(ecckey, 0);
	return retv;
}

void ecc_sign(struct ecc_sig *sig, const struct ecc_key *key,
		CBYTE *mesg, int len)
{
	struct sha256 sha;
	unsigned int dgst[ECCKEY_INT_LEN];
	unsigned int kx[ECCKEY_INT_LEN];
	mpz_t k, r, s, dst, k_inv, prikey;
	struct curve_point K;
	size_t count_r, count_s;

	sha256_reset(&sha);
	sha256(&sha, mesg, len);
	memcpy(dgst, sha.H, ECCKEY_INT_LEN*4);

	mpz_init2(dst, BITLEN);
	mpz_import(dst, ECCKEY_INT_LEN, 1, 4, 0, 0, dgst);
	mpz_init2(prikey, BITLEN);
	mpz_import(prikey, ECCKEY_INT_LEN, 1, 4, 0, 0, key->pr);

	point_init(&K);
	mpz_init2(s, 2*BITLEN);
	mpz_set_ui(s, 1);
	mpz_init2(k, BITLEN);
	mpz_init2(k_inv, BITLEN);
	mpz_init2(r, BITLEN);

	do {
		do {
			rand32bytes((unsigned char *)kx, 32, 0);
			mpz_import(k, ECCKEY_INT_LEN, 1, 4, 0, 0, kx);
		} while (mpz_cmp_ui(k, 0) == 0 || mpz_cmp(k, epn) >= 0);

		point_x_num_G(&K, k);
		mpz_mod(r, K.x, epn);
		if (mpz_cmp_ui(r, 0) == 0)
			continue;
		mpz_invert(k_inv, k, epn);

		mpz_mul(s, prikey, r);
		mpz_add(s, s, dst);
		mpz_mul(s, k_inv, s);
		mpz_mod(s, s, epn);
	} while (mpz_cmp_ui(s, 0) == 0);

	mpz_export(sig->sig_r, &count_r, 1, 4, 0, 0, r);
	mpz_export(sig->sig_s, &count_s, 1, 4, 0, 0, s); 
	assert(count_r != 0 && count_s != 0);
	point_clear(&K);
	mpz_clears(k, r, dst, k_inv, s, prikey, NULL);
}

int ecc_verify(const struct ecc_sig *sig, const struct ecc_key *key,
		CBYTE *mesg, int len)
{
	struct sha256 sha;
	unsigned int dgst[ECCKEY_INT_LEN];
	mpz_t x, s, r;
	mpz_t w, u1, u2;
	struct curve_point H, Q;
	int retv;

	retv = 0;
	mpz_init2(s, BITLEN);
	mpz_init2(r, BITLEN);
	mpz_import(s, ECCKEY_INT_LEN, 1, 4, 0, 0, sig->sig_s);
	mpz_import(r, ECCKEY_INT_LEN, 1, 4, 0, 0, sig->sig_r);
	assert(mpz_cmp(s, epn) < 0 && mpz_cmp(r, epn) < 0);

	sha256_reset(&sha);
	sha256(&sha, mesg, len);
	memcpy(dgst, sha.H, ECCKEY_INT_LEN*4);

	mpz_init2(x, BITLEN);
	mpz_import(x, ECCKEY_INT_LEN, 1, 4, 0, 0, dgst);

	mpz_init2(w, BITLEN);
	mpz_invert(w, s, epn);

	mpz_init2(u1, 2*BITLEN);
	mpz_mul(u1, x, w);
	mpz_mod(u1, u1, epn);
	mpz_init2(u2, 2*BITLEN);
	mpz_mul(u2, r, w);
	mpz_mod(u2, u2, epn);

	point_init(&H);
	point_assign(&H, key->px, key->py);
	point_x_num_nG(&H, u2, &H);
	
	point_init(&Q);
	point_x_num_G(&Q, u1);

	point_add(&H, &H, &Q);
	mpz_mod(w, H.x, epn);

	retv = mpz_cmp(w, r);
	mpz_clears(x, s, r, w, u1, u2, NULL);
	point_clear(&H);
	point_clear(&Q);
	return retv == 0;
}

int ecc_key_export(char *str, int buflen,
		const struct ecc_key *ecckey, int flag)
{
	char fm;
	int len;
	const unsigned int *key;

	if (flag == ECCKEY_EXPRIV) {
		fm = '0';
		key = ecckey->pr;
	} else if (flag == ECCKEY_EXPUB) {
		key = ecckey->px;
		if (ecckey->py[ECCKEY_INT_LEN-1] & 1)
			fm = '1';
		else
			fm = '2';
	} else
		return 1;
	*str = fm;
	len = bin2str_b64(str+1, buflen-1, (const unsigned char *)key, ECCKEY_INT_LEN*4);
	return len + 1 > buflen? -(len+1):(len+1);
}

int ecc_key_export_str(char *str, int len, const unsigned char ekey[96],
		int flag)
{
	return ecc_key_export(str, len, (const struct ecc_key *)ekey, flag);
}

int ecc_key_import(struct ecc_key *ecckey, const char *str)
{
	struct curve_point P;
	int retv = 0;

	memset(ecckey, 0, sizeof(struct ecc_key));
	switch(*str) {
	case '0':
		retv = str2bin_b64((unsigned char *)ecckey->pr, ECCKEY_INT_LEN*4, str+1);
		assert(retv == 32);
		compute_public(ecckey, 0);
		break;
	case '1':
		retv = str2bin_b64((unsigned char *)ecckey->px, ECCKEY_INT_LEN*4, str+1);
		assert(retv == 32);
		compute_public(ecckey, 1);
		break;
	case '2':
		retv = str2bin_b64((unsigned char *)ecckey->px, ECCKEY_INT_LEN*4, str+1);
		assert(retv == 32);
		compute_public(ecckey, 2);
		break;
	}
	point_init(&P);
	mpz_import(P.x, ECCKEY_INT_LEN, 1, 4, 0, 0, ecckey->px);
	mpz_import(P.y, ECCKEY_INT_LEN, 1, 4, 0, 0, ecckey->py);
	if (!is_on_curve(&P))
		retv = -65;
	point_clear(&P);
	return retv;
}

static struct curve_point n2P[256];
static int sub = 0;

int ecc_gen_table(void)
{
	if (sub == 0) {
		point_init(n2P);
		point_set(n2P, G);
		sub += 1;
	} else if (sub < 256) {
		point_init(n2P+sub);
		point_add(n2P+sub, n2P+sub-1, n2P+sub-1);
		assert(is_on_curve(n2P+sub));
		sub += 1;
	}

	return sub == 256;
}

void ecc_prn_table(void)
{
	unsigned int px[8], py[8];
	int i, j;
	size_t cx, cy;

	if (sub < 256)
		return;
	for (i = 0; i < 256; i++) {
		mpz_export(px, &cx, 1, 4, 0, 0, n2P[i].x);
		printf("	{\n");
		printf("		{\n");
		printf("			");
		for (j = 0; j < 4; j++)
			printf("%#08X, ", px[j]);
		printf("\n			");
		for (; j < 7; j++)
			printf("%#08X, ", px[j]);
		printf("%#08X\n", px[j]);
		printf("		},\n");
		printf("		{\n");
		printf("			");
		mpz_export(py, &cy, 1, 4, 0, 0, n2P[i].y);
		for (j = 0; j < 4; j++)
			printf("%#08X, ", py[j]);
		printf("\n			");
		for (; j < 7; j++)
			printf("%#08X, ", py[j]);
		printf("%#08X\n", py[j]);
		printf("		}\n");
		printf("	},\n");
	}
}

int ecc_key_hash(char *str, int buflen, const struct ecc_key *ecckey)
{
	struct ripemd160 ripe;
	int len;

	ripemd160_reset(&ripe);
	ripemd160_dgst(&ripe, (const unsigned char *)ecckey->px, ECCKEY_INT_LEN*8);
	len = bin2str_b64(str, buflen, (const unsigned char *)ripe.H, RIPEMD_LEN);
	return len;
}

int ecc_sig2str(char *buf, int buflen, const struct ecc_sig *sig)
{
	int s_len, r_len;

	s_len = 0;
	r_len = bin2str_b64(buf, buflen, (const unsigned char *)sig->sig_r, ECCKEY_INT_LEN*4);
	if (r_len < buflen) {
		buf[r_len] = ',';
		s_len = bin2str_b64(buf+r_len+1, buflen - r_len -1,
				(const unsigned char *)sig->sig_s, ECCKEY_INT_LEN*4);
	}
	if (s_len + r_len + 1 < buflen)
		buf[s_len+r_len+1] = 0;
	else
		buf[buflen-1] = 0;
	return s_len + r_len + 1;
}

int ecc_str2sig(struct ecc_sig *sig, const char *str)
{
	const char *comma, *s_str;
	char *r_str;
	int r_len, ovf = 0;

	comma = strchr(str, ',');
	if (!comma)
		return -1;
	r_len = (int)(comma - str);
	r_str = malloc(r_len+1);
	memcpy(r_str, str, r_len);
	r_str[r_len] = 0;
	ovf = str2bin_b64((unsigned char *)sig->sig_r, ECCKEY_INT_LEN*4, r_str);
	free(r_str);
	if (ovf < 0)
		return ovf;
	s_str = comma + 1;
	ovf = str2bin_b64((unsigned char *)sig->sig_s, ECCKEY_INT_LEN*4, s_str);

	return ovf;
}

int ecc_key_import_str(unsigned char ecckey[96], const char *str)
{
	        return ecc_key_import((struct ecc_key *)ecckey, str);
}

int ecc_key_hash_str(char *str, int len, const unsigned char ekey[96])
{
	        return ecc_key_hash(str, len, (const struct ecc_key *)ekey);
}
