#include <stdio.h>
#include "ecc_secp256k1.h"

int main(int argc, char *argv[])
{
	ecc_init();
	while (!ecc_gen_table())
		;
	ecc_prn_table();
	ecc_exit();
}
