#ifndef ECC_SECP256K1_DSCAO__
#define ECC_SECP256K1_DSCAO__

struct ecc_key {
	unsigned int pr[8];
	unsigned int px[8];
	unsigned int py[8];
};

void ecc_init(void);
void ecc_exit(void);

int ecc_genkey(struct ecc_key *ecckey, int secs);
void ecc_comkey(struct ecc_key *ecckey);

#endif /* ECC_SECP256K1_DSCAO__ */
