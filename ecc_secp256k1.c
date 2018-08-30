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

static void point_x_num(struct curve_point *R, mpz_t num)
{
	struct curve_point P, tp;
	unsigned int x[8], cnum;
	int i, j;
	size_t count;

	point_init(&tp);
	point_init(&P);

	mpz_set_ui(R->x, 0);
	mpz_set_ui(R->y, 0);
	mpz_set(P.x, G.x);
	mpz_set(P.y, G.y);
	mpz_export(x, &count, 1, 4, 0, 0, num);

	for (i = 7; i >= 0; i--) {
		cnum = x[i];
		for (j = 0; j < 32; j++) {
			if (cnum & 1) {
				point_add(&tp, R, &P);
				mpz_set(R->x, tp.x);
				mpz_set(R->y, tp.y);
			}
			point_double(&tp, &P);
			mpz_set(P.x, tp.x);
			mpz_set(P.y, tp.y);
			cnum >>= 1;
		}
	}
}

static void compute_public(struct ecc_key *ecckey, mpz_t x)
{
	struct curve_point P;
	size_t count_x, count_y;

	point_init(&P);
	point_x_num(&P, x);
	mpz_export(ecckey->px, &count_x, 1, 4, 0, 0, P.x);
	mpz_export(ecckey->py, &count_y, 1, 4, 0, 0, P.y);
	assert(count_x == 8 && count_y == 8);
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
