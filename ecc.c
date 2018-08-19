#include <stdio.h>
#include "ecc_secp256k1.h"

int main(int argc, char *argv[])
{
	ecc_init();
	printf("ECC initialized well!\n");
	ecc_exit();
	return 0;
}
