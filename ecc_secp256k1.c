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

static int ecc_check(void);
static int moduli_3_4(void);

void ecc_exit(void)
{
	mpz_clear(epm);
	mpz_clear(gx);
	mpz_clear(gy);
	mpz_clear(epn);
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

	assert(ecc_check() == 0);
	assert(mpz_probab_prime_p(epm, 64) != 0);
	assert(mpz_probab_prime_p(epn, 64) != 0);
	assert(moduli_3_4() == 3);
}

static int moduli_3_4(void)
{
	mpz_t r;
	unsigned long rm;

	mpz_init2(r, 256);
	rm = mpz_fdiv_r_ui(r, epm, 4);
	mpz_clear(r);
	return rm;
}

static int ecc_check(void)
{
	mpz_t tr, rcx, rcy;
	int retv;

	mpz_init2(tr, 512);
	mpz_init2(rcx, 256);
	mpz_init2(rcy, 256);

	mpz_mul(tr, gx, gx);
	mpz_fdiv_r(rcx, tr, epm);
	mpz_mul(tr, gx, rcx);
	mpz_fdiv_r(rcx, tr, epm);
	mpz_add_ui(rcx, rcx, b);

	mpz_mul(tr, gy, gy);
	mpz_fdiv_r(rcy, tr, epm);

	retv = mpz_cmp(rcx, rcy);
	mpz_clear(tr);
	mpz_clear(rcx);
	mpz_clear(rcy);

	return retv;
}
