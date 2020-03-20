#ifndef ECC_SECP256K1_DSCAO__
#define ECC_SECP256K1_DSCAO__
/*
 * Elliptic Curve Cryptography
 * Implementation of secp256k1, y exp 2 = x exp 3 + 7.
 * Dashi Cao, dscao999@hotmail.com, caods1@lenovo.com
 *
 */

#define ECCKEY_INT_LEN	8
#define ECCKEY_EXPRIV	0x81
#define ECCKEY_EXPUB	0x7e

struct ecc_key {
	unsigned int pr[ECCKEY_INT_LEN];
	unsigned int px[ECCKEY_INT_LEN];
	unsigned int py[ECCKEY_INT_LEN];
};
struct ecc_sig {
	unsigned int sig_r[ECCKEY_INT_LEN];
	unsigned int sig_s[ECCKEY_INT_LEN];
};

int ecc_sig2str(char *buf, int buflen, const struct ecc_sig *sig);
int ecc_str2sig(struct ecc_sig *sig, const char *buf);

void ecc_init(void);
void ecc_exit(void);

static inline int ecc_pubkey_only(const struct ecc_key *ekey)
{
	int i;
	for (i = 0; i < ECCKEY_INT_LEN; i++)
		if (ekey->pr[i])
			return 0;
	return 1;
}
#ifdef __linux__
int ecc_genkey(struct ecc_key *ecckey, int secs);
#endif
void ecc_writkey(const struct ecc_key *ecckey, unsigned char bt[48],
		const char *ps, int len);
int ecc_readkey(struct ecc_key *ecckey, const unsigned char bt[48],
		const char *ps, int len);

int ecc_key_export(char *str, int len, const struct ecc_key *ecckey, int flag);
int ecc_key_export_str(char *str, int len, const unsigned char ekey[96],
		int flag);

int ecc_key_import(struct ecc_key *ecckey, const char *str);
int ecc_key_import_str(unsigned char ecckey[96], const char *str);

int ecc_key_hash(char *str, int len, const struct ecc_key *ecckey);
int ecc_key_hash_str(char *str, int len, const unsigned char ekey[96]);

void ecc_sign(struct ecc_sig *sig, const struct ecc_key *key,
		const unsigned char *mesg, int len);
void ecc_sign_pronly(struct ecc_sig *sig, const unsigned char *prkey,
		const unsigned char *mesg, int len);

int ecc_verify(const struct ecc_sig *sig, const struct ecc_key *key,
		const unsigned char *mesg, int len);

int ecc_gen_table(void);
void ecc_prn_table(void);
#endif /* ECC_SECP256K1_DSCAO__ */
