/*
 * y exp 2 = x exp 3 + 7
 */
#include <gmp.h>
#include <assert.h>
#include "ecc_secp256k1.h"

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
static mpz_t gx, gy;
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
	mpz_clear(gx);
	mpz_clear(gy);
	mpz_clear(epn);
	mpz_clear(sroot);
}

void ecc_init(void)
{
	mpz_init2(epm, 256);
	mpz_import(epm, 8, 1, 4, 0, 0, EPM);
	mpz_init2(gx, 256);
	mpz_import(gx, 8, 1, 4, 0, 0, GX);
	mpz_init2(gy, 256);
	mpz_import(gy, 8, 1, 4, 0, 0, GY);
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

	mpz_init2(tr, 512);
	mpz_init2(rcx, 512);

	mpz_mul(rcx, gx, gx);
	mpz_mod(rcx, rcx, epm);
	mpz_mul(rcx, gx, rcx);
	mpz_mod(rcx, rcx, epm);
	mpz_add_ui(rcx, rcx, b);
	a_sroot(tr, rcx);
	retv = mpz_cmp(tr, gy);
	
	mpz_clear(tr);
	mpz_clear(rcx);
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
