#ifndef RIPEMD160_DGST_DSCAO__
#define RIPEMD160_DGST_DSCAO__
#include <stdlib.h>

struct ripemd160 {
	unsigned int H[5];
};

static inline struct ripemd160 *ripemd160_init(void)
{
	struct ripemd160 *ripe;

	ripe = malloc(sizeof(struct ripemd160));
	if (ripe) {
		ripe->H[0] = 0x67452301;
		ripe->H[1] = 0xEFCDAB89;
		ripe->H[2] = 0x98BADCFE;
		ripe->H[3] = 0x10325476;
		ripe->H[4] = 0xC3D2E1F0;
	}
	return ripe;
}
		
static inline void ripemd160_exit(struct ripemd160 *ripe)
{
	free(ripe);
}

void ripemd160_dgst(struct ripemd160 *ripe, const unsigned char *msg, int len);
#endif /* RIPEMD160_DGST_DSCAO__ */
