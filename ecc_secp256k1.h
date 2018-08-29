#ifndef ECC_SECP256K1_DSCAO__
#define ECC_SECP256K1_DSCAO__

struct ecc_key {
	unsigned char pr[32];
	unsigned char px[32];
	unsigned char py[32];
} __attribute__((aligned(4)));

void ecc_init(void);
void ecc_exit(void);

int ecc_genkey(struct ecc_key *ecckey, int secs);

#endif /* ECC_SECP256K1_DSCAO__ */
