#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "dsaes.h"

static void suite_test_1(FILE *tf)
{
	unsigned int knum, i;
	struct aeskey *w;
	unsigned char key[16];
	unsigned char plain[16];
	unsigned char cipher[16];
	unsigned char decipr[16];

	for (i = 0; i < 16; i++) {
		fscanf(tf, "%2x", &knum);
		key[i] = knum;
	}
	printf("  Key: ");
	for (i = 0; i < 16; i++)
		printf("%02X", (unsigned int)key[i]);
	printf("\n");
	w = aes_init(key);

	do {
		for (i = 0; i < 16; i++) {
			fscanf(tf, "%2x", &knum);
			plain[i] = knum;
		}
		if (feof(tf))
			break;
		dsaes(w, plain, cipher, 16);
		printf("Plain: ");
		for (i = 0; i < 16; i++)
			printf("%02X", (unsigned int)plain[i]);
		printf("|");
		for (i = 0; i < 16; i++)
			printf("%02X", (unsigned int)cipher[i]);
		printf("\n");
		un_dsaes(w, cipher, decipr, 16);
		printf("Compare: %d\n", memcmp(plain, decipr, 16));
		while (!feof (tf) && fgetc(tf) != '\n')
			;
	} while (1);

	aes_exit(w);
}

static void suite_test_2(FILE *tf)
{
	unsigned int knum, i;
	struct aeskey *w;
	unsigned char key[16];
	unsigned char plain[16];
	unsigned char cipher[16];
	unsigned char decipr[16];

	memset(plain, 0, 16);

	do {
		for (i = 0; i < 16; i++) {
			fscanf(tf, "%2x", &knum);
			key[i] = knum;
		}
		if (feof(tf))
			break;
		printf("  Key: ");
		for (i = 0; i < 16; i++)
			printf("%02X", (unsigned int)key[i]);
		w = aes_init(key);

		dsaes(w, plain, cipher, 16);
		printf("|");
		for (i = 0; i < 16; i++)
			printf("%02X", (unsigned int)cipher[i]);
		printf("\n");
		un_dsaes(w, cipher, decipr, 16);
		printf("Compare: %d\n", memcmp(plain, decipr, 16));
		while (!feof (tf) && fgetc(tf) != '\n')
			;
		aes_exit(w);
	} while (1);

}

static void suite_test_3(FILE *tf)
{
	unsigned int i;
	struct aeskey *w;
	unsigned char key[16];
	unsigned char plain[16];
	unsigned char cipher[16];
	unsigned char decipr[16];

	memset(key, 0, 16);
	w = aes_init(key);
	memset(plain, 0, 16);
	plain[0] = 0x80;

	do {
		dsaes(w, plain, cipher, 16);
		printf("Plain: ");
		for (i = 0; i < 16; i++)
			printf("%02X", (unsigned int)plain[i]);
		printf("|");
		for (i = 0; i < 16; i++)
			printf("%02X", (unsigned int)cipher[i]);
		printf("\n");
		un_dsaes(w, cipher, decipr, 16);
		printf("Compare: %d\n", memcmp(plain, decipr, 16));
		for (i = 15; i > 0; i--)
			plain[i] = (plain[i] >> 1)| ((plain[i-1] & 1) << 7);
		plain[0] = (plain[0] >> 1) | 0x80;
	} while (plain[15] != 0xff);
	dsaes(w, plain, cipher, 16);
	printf("Plain: ");
	for (i = 0; i < 16; i++)
		printf("%02X", (unsigned int)plain[i]);
	printf("|");
	for (i = 0; i < 16; i++)
		printf("%02X", (unsigned int)cipher[i]);
	printf("\n");
	un_dsaes(w, cipher, decipr, 16);
	printf("Compare: %d\n", memcmp(plain, decipr, 16));

	aes_exit(w);
}

int main(int argc, char *argv[])
{
	int suite;
	FILE *tf;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s test-file suite\n", argv[0]);
		return 4;
	}
	suite = atoi(argv[2]);
	if (suite == 3)
		tf = fopen(argv[1], "wb");
	else
		tf = fopen(argv[1], "rb");
	switch(suite) {
	case 1:
		suite_test_1(tf);
		break;
	case 2:
		suite_test_2(tf);
		break;
	case 3:
		suite_test_3(tf);
		break;
	default:
		assert(0);
	}

	return 0;
}
