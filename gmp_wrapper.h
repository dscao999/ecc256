#ifndef GMP_WRAPPER_DSCAO__
#define GMP_WRAPPER_DSCAO__

static inline void mpz_mul_mod(mpz_t rop, const mpz_t x, const mpz_t y)
{
	mpz_mul(rop, x, y);
	mpz_mod(rop, rop, epm);
}

static inline void mpz_mul_ui_mod(mpz_t rop, const mpz_t x, unsigned long y)
{
	mpz_mul_ui(rop, x, y);
	mpz_mod(rop, rop, epm);
}

static inline void mpz_add_mod(mpz_t rop, const mpz_t x, const mpz_t y)
{
	mpz_add(rop, x, y);
	mpz_mod(rop, rop, epm);
}

static inline void mpz_add_ui_mod(mpz_t rop, const mpz_t x, unsigned long y)
{
	mpz_add_ui(rop, x, y);
	mpz_mod(rop, rop, epm);
}

static inline void mpz_sub_mod(mpz_t rop, const mpz_t x, const mpz_t y)
{
	mpz_sub(rop, x, y);
	mpz_mod(rop, rop, epm);
}
#endif /* GMP_WRAPPER_DSCAO__ */
