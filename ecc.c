#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "ecc_secp256k1.h"
#include "dscrc.h"

int main(int argc, char *argv[])
{
	struct ecc_key mkey, tkey;
	unsigned int key_crc;
	FILE *ko;
	struct stat kfstat;
	int sysret, len;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s keyfile\n", argv[0]);
		return 4;
	}
	ecc_init();
	sysret = stat(argv[1], &kfstat);
	len = sizeof(struct ecc_key);
	if (sysret == -1 || !S_ISREG(kfstat.st_mode) ||
			(kfstat.st_mode & (S_IRUSR|S_IWUSR)) == 0 ||
			kfstat.st_size != len + 4) {
		ko = fopen(argv[1], "wb");
		if (!ko) {
			fprintf(stderr, "Cannot open file: %s for writting.\n",
				argv[1]);
			return 8;
		}
		ecc_genkey(&mkey, 5);
		key_crc = crc32((unsigned char *)&mkey, sizeof(struct ecc_key));
		fwrite(&mkey, 1, len, ko);
		fwrite(&key_crc, sizeof(key_crc), 1, ko);
		fclose(ko);
	} else {
		ko = fopen(argv[1], "rb");
		if (!ko) {
			fprintf(stderr, "Cannot open file: %s for reading.\n",
				argv[1]);
			return 12;
		}
		fread(&mkey, 1, len, ko);
		fread(&key_crc, sizeof(key_crc), 1, ko);
		fclose(ko);
		if (!crc32_confirm((unsigned char *)&mkey,
				sizeof(struct ecc_key), key_crc)) {
			fprintf(stderr, "Key corrupted!\n");
			return 16;
		}
		memcpy(tkey.pr, mkey.pr, sizeof(mkey.pr));
		ecc_comkey(&tkey);
		if (memcmp(tkey.px, mkey.px, sizeof(mkey.px)) != 0 ||
				memcmp(tkey.py, mkey.py, sizeof(mkey.px)) != 0) {
			fprintf(stderr, "Key corrupted!\n");
			return 20;
		}
	}
	ecc_exit();
	return 0;
}
