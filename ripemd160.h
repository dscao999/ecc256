#ifndef RIPEMD160_DGST_DSCAO__
#define RIPEMD160_DGST_DSCAO__
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "loglog.h"

#define RIPEMD_LEN	20

struct ripemd160 {
	unsigned int H[RIPEMD_LEN/4];
};

static inline void ripemd160_reset(struct ripemd160 *ripe)
{
	if (ripe) {
		ripe->H[0] = 0x67452301;
		ripe->H[1] = 0xEFCDAB89;
		ripe->H[2] = 0x98BADCFE;
		ripe->H[3] = 0x10325476;
		ripe->H[4] = 0xC3D2E1F0;
	}
}

static inline struct ripemd160 *ripemd160_init(void)
{
	struct ripemd160 *ripe;

	ripe = malloc(sizeof(struct ripemd160));
	if (ripe)
		ripemd160_reset(ripe);
	return ripe;
}
		
static inline void ripemd160_exit(struct ripemd160 *ripe)
{
	if (ripe)
		free(ripe);
}

void ripemd160_dgst(struct ripemd160 *ripe, const unsigned char *msg, int len);
static inline void ripemd160_dgst_2str(unsigned char dgst[RIPEMD_LEN],
		const unsigned char *msg, int len)
{
	struct ripemd160 ripe;
	unsigned int *H;
	int i;

	assert((((unsigned long)dgst >> 2) << 2) == (unsigned long)dgst);
	H = (unsigned int *)dgst;
	ripemd160_reset(&ripe);
	ripemd160_dgst(&ripe, msg, len);
	for (i = 0; i < 5; i++)
		*H++ = swap32(ripe.H[0]);
}

void ripemd160_fdgst(struct ripemd160 *ripe, FILE *fin);
#endif /* RIPEMD160_DGST_DSCAO__ */
