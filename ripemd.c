#include <stdio.h>
#include <string.h>
#include "ripemd160.h"

int main(int argc, const char *argv[])
{
	const char *msg;
	struct ripemd160 *ripe;
	int i;

	if (argc > 1)
		msg = argv[1];
	else
		msg = "";
	ripe = ripemd160_init(NULL);
	ripemd160_dgst(ripe, (const unsigned char *)msg, strlen(msg));
	for (i = 0; i < 5; i++) {
		printf("%02x", ripe->H[i] & 0x0ff);
		printf("%02x", (ripe->H[i] >> 8) & 0x0ff);
		printf("%02x", (ripe->H[i] >> 16) & 0x0ff);
		printf("%02x", (ripe->H[i] >> 24) & 0x0ff);
	}
	printf("\n");
	ripemd160_exit(ripe);
	return 0;
}
