/*
 * y exp 2 = x exp 3 + 7
 */
#include <gmp.h>
#include <assert.h>
#include "ecc_secp256k1.h"
#include "alsa_random.h"

struct curve_point {
	mpz_t x;
	mpz_t y;
};

static void point_x_num(struct curve_point *R, mpz_t num);

static void point_init(struct curve_point *p)
{
	mpz_init2(p->x, 512);
	mpz_init2(p->y, 512);
}
static void point_exit(struct curve_point *p)
{
	mpz_clear(p->x);
	mpz_clear(p->y);
}

static const unsigned int EPM[] = {
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFE, 0xFFFFFC2F
};
static mpz_t epm;
static const unsigned short a = 0;
static const unsigned short b = 7;
static const unsigned short h = 1;
static const unsigned int GX[] = {
	0x79BE667E, 0xF9DCBBAC, 0x55A06295, 0xCE870B07, 0x029BFCDB, 0x2DCE28D9,
	0x59F2815B, 0x16F81798
} ;
static const unsigned int GY[] = {
	0x483ADA77, 0x26A3C465, 0x5DA4FBFC, 0x0E1108A8, 0xFD17B448, 0xA6855419,
	0x9C47D08F, 0xFB10D4B8
};
static struct curve_point G;

static const unsigned int EPN[] = {
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE, 0xBAAEDCE6, 0xAF48A03B,
	0xBFD25E8C, 0xD0364141
};
static mpz_t epn;
static mpz_t sroot;

static int ecc_check(void);
static int moduli_3_4(void);
static void a_exp(mpz_t x, const mpz_t a, const mpz_t e);
static inline void a_sroot(mpz_t x, const mpz_t a)
{
	a_exp(x, a, sroot);
}

void ecc_exit(void)
{
	mpz_clear(epm);
	point_exit(&G);
	mpz_clear(epn);
	mpz_clear(sroot);
}

void ecc_init(void)
{
	mpz_init2(epm, 256);
	mpz_import(epm, 8, 1, 4, 0, 0, EPM);
	point_init(&G);
	mpz_import(G.x, 8, 1, 4, 0, 0, GX);
	mpz_import(G.y, 8, 1, 4, 0, 0, GY);
	mpz_init2(epn, 256);
	mpz_import(epn, 8, 1, 4, 0, 0, EPN);

	mpz_init2(sroot, 256);
	mpz_add_ui(sroot, epm, 1);
	mpz_fdiv_q_2exp(sroot, sroot, 2);

	assert(mpz_probab_prime_p(epm, 64) != 0);
	assert(mpz_probab_prime_p(epn, 64) != 0);
	assert(moduli_3_4() == 3);
	assert(ecc_check() == 0);
}

static int moduli_3_4(void)
{
	mpz_t r;
	int rm;

	mpz_init2(r, 256);
	rm = mpz_mod_ui(r, epm, 4);
	mpz_clear(r);
	return rm;
}

static int ecc_check(void)
{
	mpz_t tr, rcx;
	int retv;
	struct curve_point point;
	unsigned int x[8], y[8];
	size_t count_x, count_y;

	mpz_init2(tr, 512);
	mpz_init2(rcx, 512);

	mpz_mul(rcx, G.x, G.x);
	mpz_mod(rcx, rcx, epm);
	mpz_mul(rcx, G.x, rcx);
	mpz_mod(rcx, rcx, epm);
	mpz_add_ui(rcx, rcx, b);
	a_sroot(tr, rcx);
	retv = mpz_cmp(tr, G.y);

	point_init(&point);
	point_x_num(&point, epn);
	mpz_export(x, &count_x, 1, 4, 0, 0, point.x);
	mpz_export(y, &count_y, 1, 4, 0, 0, point.y);
	if (count_x != 0 || count_y != 0)
		retv = 1;
	
	mpz_clear(tr);
	mpz_clear(rcx);
	point_exit(&point);
	return retv;
}

static void a_exp(mpz_t x, const mpz_t a, const mpz_t e)
{
	unsigned int w[8], cw;
	unsigned long rlen;
	mpz_t fct;
	int i, j;

	mpz_init2(fct, 512);
	mpz_set_ui(x, 1);
	mpz_set(fct, a);
	mpz_export(w, &rlen, 1, 4, 0, 0, e);
	for (i = rlen-1; i >= 0; i--) {
		cw = w[i];
		for (j = 0; j < 32; j++) {
			if ((cw & 1) == 1) {
				mpz_mul(x, x, fct);
				mpz_mod(x, x, epm);
			}
			cw >>= 1;
			mpz_mul(fct, fct, fct);
			mpz_mod(fct, fct, epm);
		}
	}
	mpz_clear(fct);
}

static int point_lambd(mpz_t lambd, const struct curve_point *P,
		const struct curve_point *Q)
{
	mpz_t ztmp, wtmp;
	int retv;

	retv = 0;
	mpz_init2(ztmp, 512);
	mpz_init2(wtmp, 512);
	mpz_sub(ztmp, Q->y, P->y);
	mpz_sub(wtmp, Q->x, P->x);
	mpz_mod(wtmp, wtmp, epm);
	if (mpz_cmp_ui(wtmp, 0) != 0) {
		retv = mpz_invert(wtmp, wtmp, epm);
		assert(retv != 0);
		mpz_mul(lambd, ztmp, wtmp);
		mpz_mod(lambd, lambd, epm);
	}

	mpz_clear(ztmp);
	mpz_clear(wtmp);
	return retv;
}

static void tagent_lambd(mpz_t lambd, const struct curve_point *P)
{
	mpz_t ztmp;
	int retv;

	mpz_init2(ztmp, 512);

	mpz_mul(lambd, P->x, P->x);
	mpz_mul_ui(lambd, lambd, 3);
	mpz_mul_ui(ztmp, P->y, 2);
	retv = mpz_invert(ztmp, ztmp, epm);
	assert(retv != 0);
	mpz_mul(lambd, lambd, ztmp);
	mpz_mod(lambd, lambd, epm);

	mpz_clear(ztmp);
}

static void point_add_lambd(struct curve_point *R, const mpz_t lambd,
		const struct curve_point *P, const struct curve_point *Q)
{
	mpz_t ztmp;

	mpz_init2(ztmp, 512);

	mpz_mul(ztmp, lambd, lambd);
	mpz_sub(ztmp, ztmp, P->x);
	mpz_sub(R->x, ztmp, Q->x);
	mpz_mod(R->x, R->x, epm);

	mpz_sub(ztmp, P->x, R->x);
	mpz_mul(ztmp, ztmp, lambd);
	mpz_sub(R->y, ztmp, P->y);
	mpz_mod(R->y, R->y, epm);

	mpz_clear(ztmp);
}

static void point_add(struct curve_point *R, const struct curve_point *P,
			const struct curve_point *Q)
{
	mpz_t lambd;

	if (mpz_cmp_ui(P->x, 0) == 0 && mpz_cmp_ui(P->y, 0) == 0) {
		mpz_set(R->x, Q->x);
		mpz_set(R->y, Q->y);
	} else if (mpz_cmp_ui(Q->x, 0) == 0 && mpz_cmp_ui(Q->y, 0) == 0) {
		mpz_set(R->x, P->x);
		mpz_set(R->y, P->y);
	} else {
		mpz_init2(lambd, 512);
		if (point_lambd(lambd, P, Q))
			point_add_lambd(R, lambd, P, Q);
		else {
			mpz_set_ui(R->x, 0);
			mpz_set_ui(R->y, 0);
		}
		mpz_clear(lambd);
	}
}

static void point_double(struct curve_point *R, const struct curve_point *P)
{
	mpz_t lambd;

	mpz_init2(lambd, 512);
	tagent_lambd(lambd, P);
	point_add_lambd(R, lambd, P, P);
	mpz_clear(lambd);
}

static void point_x_num_ng(struct curve_point *R, mpz_t num,
		const struct curve_point *P)
{
	struct curve_point tp, S;
	unsigned int x[8], cnum;
	size_t count;
	int i, j;

	mpz_set_ui(R->x, 0);
	mpz_set_ui(R->y, 0);
	point_init(&S);
	mpz_set(S.x, P->x);
	mpz_set(S.y, P->y);

	mpz_export(x, &count, 1, 4, 0, 0, num);
	point_init(&tp);
	for (i = count-1; i >= 0; i--) {
		cnum = x[i];
		for (j = 0; j < 32; j++) {
			if (cnum & 1) {
				point_add(&tp, R, &S);
				mpz_set(R->x, tp.x);
				mpz_set(R->y, tp.y);
			}
			point_double(&tp, &S);
			mpz_set(S.x, tp.x);
			mpz_set(S.y, tp.y);
			cnum >>= 1;
		}
	}
	point_exit(&tp);
	point_exit(&S);
}

static void point_x_num(struct curve_point *R, mpz_t num)
{
	struct curve_point P;

	point_init(&P);
	mpz_set(P.x, G.x);
	mpz_set(P.y, G.y);

	point_x_num_ng(R, num, &P);

	point_exit(&P);
}

static void compute_public(struct ecc_key *ecckey, mpz_t x)
{
	struct curve_point P;
	size_t count_x, count_y;

	point_init(&P);
	point_x_num(&P, x);
	mpz_export(ecckey->px, &count_x, 1, 4, 0, 0, P.x);
	mpz_export(ecckey->py, &count_y, 1, 4, 0, 0, P.y);
	assert(count_x != 0 && count_y != 0);
	point_exit(&P);
}

int ecc_genkey(struct ecc_key *ecckey, int secs)
{
	int retv, len;
	struct alsa_param *alsa;
	mpz_t x;

	secs = secs <= 0? 1 : secs;
	len = secs * SAMPLE_HZ * 4;
	alsa = alsa_init(len);
	if (!alsa)
		return 10000;
	mpz_init2(x, 256);

	do {
		retv = alsa_random(alsa, ecckey->pr);
		mpz_import(x, 8, 1, 4, 0, 0, ecckey->pr);
	} while (mpz_cmp(x, epn) >= 0);

	compute_public(ecckey, x);

	mpz_clear(x);
	alsa_exit(alsa);

	return retv;
}

void ecc_comkey(struct ecc_key *ecckey)
{
	mpz_t x;

	mpz_init2(x, 256);
	mpz_import(x, 8, 1, 4, 0, 0, ecckey->pr);
	compute_public(ecckey, x);
	mpz_clear(x);
}

void ecc_sign(struct ecc_sig *sig, const struct ecc_key *key,
		const unsigned char *mesg, int len)
{
	struct sha256_handle *sha;
	unsigned int dgst[8];
	unsigned int kx[8];
	struct alsa_param *alsa;
	mpz_t k, r, dst, k_inv, s, skey;
	struct curve_point kg;
	size_t count_r, count_s;

	sha = sha256_init();
	sha256(sha, mesg, len, dgst);
	sha256_exit(sha);
	mpz_init2(dst, 256);
	mpz_import(dst, 8, 1, 4, 0, 0, dgst);
	mpz_init2(skey, 256);
	mpz_import(skey, 8, 1, 4, 0, 0, key->pr);

	mpz_init2(s, 512);
	point_init(&kg);
	mpz_init2(k, 256);
	mpz_init2(r, 256);
	alsa = alsa_init(1);

	do {
		do {
			alsa_random(alsa, kx);
			mpz_import(k, 8, 1, 4, 0, 0, kx);
		} while (mpz_cmp(k, epn) >= 0);

		point_x_num(&kg, k);
		mpz_mod(r, kg.x, epn);
		if (mpz_cmp_ui(r, 0) == 0)
			continue;
		mpz_invert(k_inv, k, epn);

		mpz_mul(s, skey, r);
		mpz_add(s, s, dst);
		mpz_mul(s, k_inv, s);
		mpz_mod(s, s, epn);
	} while (mpz_cmp_ui(s, 0) == 0);

	alsa_exit(alsa);

	mpz_export(sig->sig_r, &count_r, 1, 4, 0, 0, r);
	mpz_export(sig->sig_s, &count_s, 1, 4, 0, 0, s); 
	assert(count_r != 0 && count_s != 0);
}

int ecc_verify(const struct ecc_sig *sig, const struct ecc_key *key,
		const unsigned char *mesg, int len)
{
	struct sha256_handle *sha;
	unsigned int dgst[8];
	mpz_t X, s, r;
	mpz_t w, u1, u2;
	struct curve_point H, Q, tp;
	int retv;

	retv = 0;
	mpz_init2(s, 256);
	mpz_init2(r, 256);
	mpz_import(s, 8, 1, 4, 0, 0, sig->sig_s);
	mpz_import(r, 8, 1, 4, 0, 0, sig->sig_r);
	if (mpz_cmp(s, epn) >= 0 || mpz_cmp(r, epn) >= 0)
		return 0;

	sha = sha256_init();
	sha256(sha, mesg, len, dgst);
	sha256_exit(sha);

	mpz_init2(X, 256);
	mpz_import(X, 8, 1, 4, 0, 0, dgst);

	mpz_init2(w, 256);
	mpz_invert(w, s, epn);

	mpz_init2(u1, 512);
	mpz_mul(u1, X, w);
	mpz_mod(u1, u1, epn);
	mpz_init2(u2, 512);
	mpz_mul(u2, r, w);
	mpz_mod(u2, u2, epn);

	point_init(&H);
	mpz_import(H.x, 8, 1, 4, 0, 0, key->px);
	mpz_import(H.y, 8, 1, 4, 0, 0, key->py);
	point_init(&tp);
	point_x_num_ng(&tp, u2, &H);
	
	point_init(&Q);
	point_x_num(&Q, u1);

	point_add(&H, &tp, &Q);
	mpz_mod(w, H.x, epn);

	retv = mpz_cmp(w, r);
	return retv == 0;
}
