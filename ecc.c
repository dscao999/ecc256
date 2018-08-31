#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include "ecc_secp256k1.h"
#include "dscrc.h"

static inline int malloc_len(int len)
{
	return ((((len - 1) >> 4) + 1) << 4);
}

#define GEN_KEY	0x01
#define SIGN_FILE	0x02
#define SIG_VERIFY	0x04

static int key_process(struct ecc_key *mkey, const char *keyfile, int action)
{
	unsigned int key_crc;
	FILE *ko;

	if (action & GEN_KEY) {
		ko = fopen(keyfile, "wb");
		if (!ko) {
			fprintf(stderr, "Cannot open key file %s for writing!\n",
					keyfile);
			return 4;
		}
		ecc_genkey(mkey, 5);
		key_crc = crc32((unsigned char *)mkey, sizeof(struct ecc_key));
		fwrite(mkey, sizeof(struct ecc_key), 1, ko);
		fwrite(&key_crc, sizeof(key_crc), 1, ko);
		fclose(ko);
	} else {
		ko = fopen(keyfile, "rb");
		if (!ko) {
			fprintf(stderr, "Cannot open file: %s for reading.\n",
				keyfile);
			return 12;
		}
		fread(mkey, sizeof(struct ecc_key), 1, ko);
		fread(&key_crc, sizeof(key_crc), 1, ko);
		fclose(ko);
		if (!crc32_confirm((unsigned char *)mkey,
				sizeof(struct ecc_key), key_crc)) {
			fprintf(stderr, "Key corrupted!\n");
			return 16;
		}
	}

	return 0;
}

static int sign_file(const char *msgfile, const struct ecc_key *mkey, const char *sigfile)
{
	FILE *mi;
	void *area;
	char *mesg;
	struct stat mstat;
	struct ecc_sig *sig;
	unsigned int crc;
	int sysret, retv, len;

	retv = 0;
	sysret = stat(msgfile, &mstat);
	if (sysret == -1) {
		fprintf(stderr, "File %s error: %s\n", msgfile, strerror(errno));
		return 8;
	}
	len = malloc_len(mstat.st_size);
	area = malloc(len+sizeof(struct ecc_sig));
	if (!area) {
		fprintf(stderr, "Out of Memory!\n");
		return 10000;
	}
	mesg = area;
	sig =  area + len;

	mi = fopen(msgfile, "rb");
	if (!mi) {
		fprintf(stderr, "Cannot openfile %s for reading.\n",
				msgfile);
		retv = 4;
		goto exit_10;
	}
	sysret = fread(mesg, 1, mstat.st_size, mi);
	if (sysret != mstat.st_size) {
		fprintf(stderr, "Read file %s error\n", msgfile);
		retv = 12;
		goto exit_20;
	}
	fclose(mi);
	mi = NULL;

	ecc_sign(sig, mkey, (unsigned char *)mesg, mstat.st_size);

	mi = fopen(sigfile, "wb");
	if (!mi) {
		fprintf(stderr, "Cannot open file %s for writing.\n",
			sigfile);
		retv = 4;
		goto exit_20;
	}
	sysret = fwrite(sig, sizeof(struct ecc_sig), 1, mi);
	if (sysret != 1)
		fprintf(stderr, "Write error %s: %s\n", sigfile, strerror(errno));
	crc = crc32((unsigned char *)sig, sizeof(struct ecc_sig));
	sysret = fwrite(&crc, sizeof(crc), 1, mi);
	fclose(mi);
	mi = NULL;

exit_20:
	if (mi)
		fclose(mi);
exit_10:
	free(area);
	return retv;
}

static int verify_file(const char *msgfile, const struct ecc_key *mkey,
		const char *sigfile)
{
	FILE *mi;
	int retv, sysret;
	struct stat fst;
	void *area;
	unsigned char *mesg;
	struct ecc_sig *sig;
	unsigned int crc;
	unsigned long len;

	retv = 0;
	sysret = stat(msgfile, &fst);
	if (sysret == -1) {
		fprintf(stderr, "File %s error: %s\n", msgfile,
			strerror(errno));
		return -1;
	}
	mi = fopen(msgfile, "rb");
	if (!mi) {
		fprintf(stderr, "Cannot open file %s for read.\n", msgfile);
		return -1;
	}
	area = malloc(malloc_len(fst.st_size)+sizeof(struct ecc_sig));
	if (!area) {
		retv = -10000;
		goto exit_10;
	}
	len = fst.st_size;
	sig = area + malloc_len(fst.st_size);
	mesg = area;
	sysret = fread(mesg, 1, fst.st_size, mi);
	if (sysret != fst.st_size) {
		fprintf(stderr, "File read error!\n");
		goto exit_10;
	}
	fclose(mi);
	mi = NULL;

	sysret = stat(sigfile, &fst);
	if (sysret == -1) {
		fprintf(stderr, "File %s error: %s\n", sigfile,
			strerror(errno));
		retv = -1;
		goto exit_20;
	}
	if (fst.st_size != 68) {
		fprintf(stderr, "Invalid Signature %s\n", sigfile);
		goto exit_20;
	}
	mi = fopen(sigfile, "rb");
	if (!mi) {
		fprintf(stderr, "Cannot open file %s for reading.\n", sigfile);
		retv = -4;
		goto exit_20;
	}
	fread(sig, sizeof(struct ecc_sig), 1, mi);
	fread(&crc, sizeof(crc), 1, mi);
	fclose(mi);
	mi = NULL;
	if (!crc32_confirm((unsigned char *)sig, sizeof(struct ecc_sig), crc)) {
		fprintf(stderr, "Corrupted signature!\n");
		goto exit_20;
	}
	retv = ecc_verify(sig, mkey, mesg, len);

exit_20:
	if (area)
		free(area);
exit_10:
	if (mi)
		fclose(mi);
	return retv;
}

int main(int argc, char *argv[])
{
	struct ecc_key *mkey;
	void *buffer;
	int fin, opt, action, retv;
	const char *keyfile, *msgfile, *sigfile;
	extern int optind, opterr, optopt;
	extern char *optarg;

	retv = 0;
	keyfile = "ecc_key.dat";
	msgfile = NULL;
	sigfile = NULL;
	opterr = 0;
	fin = 0;
	action = 0;
	do {
		opt = getopt(argc, argv, ":k:s:v:g");
		switch(opt) {
		case -1:
			fin = 1;
			break;
		case ':':
			fprintf(stderr, "Missing argument for %c\n", optopt);
			break;
		case '?':
			fprintf(stderr, "Unknown option: %c\n", optopt);
			break;
		case 'k':
			keyfile = optarg;
			break;
		case 's':
			action |= SIGN_FILE;
			sigfile = optarg;
			break;
		case 'v':
			action |= SIG_VERIFY;
			sigfile = optarg;
			break;
		case 'g':
			action |= GEN_KEY;
			break;
		default:
			assert(0);
		}
	} while (fin == 0);
	if (optind != argc && optind + 1 != argc) {
		fprintf(stderr, "Usage: %s -k keyfile [-g] [-s|-v] signfile"
				" file\n", argv[0]);
		return 10;
	}

	buffer = malloc(sizeof(struct ecc_key));
	if (!buffer) {
		fprintf(stderr, "Out of Memory!\n");
		return 10000;
	}
	mkey = buffer;
	ecc_init();

	key_process(mkey, keyfile, action);

	msgfile = argv[optind];
	if (msgfile == NULL) {
		if (action & (SIG_VERIFY|SIGN_FILE)) {
			fprintf(stderr, "Missing File!\n");
			retv = 36;
		}
		goto exit_10;
	}

	if (action & SIGN_FILE)
		sign_file(msgfile, mkey, sigfile);

	if (action & SIG_VERIFY)
		if (!verify_file(msgfile, mkey, sigfile))
			fprintf(stderr, "Signuature verification failed!\n");

exit_10:
	return retv;
}
