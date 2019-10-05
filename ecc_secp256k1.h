#ifndef ECC_SECP256K1_DSCAO__
#define ECC_SECP256K1_DSCAO__
/*
 * Elliptic Curve Cryptography
 * Implementation of secp256k1, y exp 2 = x exp 3 + 7.
 * Dashi Cao, dscao999@hotmail.com, caods1@lenovo.com
 *
 */

#define ECCKEY_LEN	8
#define ECCKEY_PRI	0x8000
#define ECCKEY_PUB	0x4000
#define ECCKEY_BRIEF	0x0100

struct ecc_key {
	unsigned int pr[ECCKEY_LEN];
	unsigned int px[ECCKEY_LEN];
	unsigned int py[ECCKEY_LEN];
};
struct ecc_sig {
	unsigned int sig_r[ECCKEY_LEN];
	unsigned int sig_s[ECCKEY_LEN];
};

void ecc_init(void);
void ecc_exit(void);

int ecc_genkey(struct ecc_key *ecckey, int secs);
int ecc_key_export(char *str, int len, const struct ecc_key *ecckey, int flag);
int ecc_key_import(struct ecc_key *ecckey, const char *str);
void ecc_comkey(struct ecc_key *ecckey);

void ecc_sign(struct ecc_sig *sig, const struct ecc_key *key,
		const unsigned char *mesg, int len);

int ecc_verify(const struct ecc_sig *sig, const struct ecc_key *key,
		const unsigned char *mesg, int len);

int ecc_gen_table(void);
void ecc_prn_table(void);
#endif /* ECC_SECP256K1_DSCAO__ */
