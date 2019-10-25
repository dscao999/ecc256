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

#define GEN_KEY		0x01
#define SIGN_FILE	0x02
#define SIG_VERIFY	0x04
#define EXPORT_KEY	0x80
#define IMPORT_KEY	0x40

static int key_process(struct ecc_key *mkey, const char *keyfile, int action,
		const char *keystr, const char *sdname)
{
	unsigned int key_crc;
	FILE *ko;

	if ((action & GEN_KEY) || (action & IMPORT_KEY))
		ko = fopen(keyfile, "wb");
	else
		ko = fopen(keyfile, "rb");
	if (!ko) {
		fprintf(stderr, "Cannot open key file %s for read/write!\n",
				keyfile);
		return 4;
	}
	if (action & GEN_KEY) {
		ecc_genkey(mkey, 5, sdname);
		key_crc = crc32((unsigned char *)mkey, sizeof(struct ecc_key));
		fwrite(mkey, sizeof(struct ecc_key), 1, ko);
		fwrite(&key_crc, sizeof(key_crc), 1, ko);
	} else if (action & IMPORT_KEY) {
		ecc_key_import(mkey, keystr);
		key_crc = crc32((unsigned char *)mkey, sizeof(struct ecc_key));
		fwrite(mkey, sizeof(struct ecc_key), 1, ko);
		fwrite(&key_crc, sizeof(key_crc), 1, ko);
	} else {
		fread(mkey, sizeof(struct ecc_key), 1, ko);
		fread(&key_crc, sizeof(key_crc), 1, ko);
		if (!crc32_check((unsigned char *)mkey,
				sizeof(struct ecc_key), key_crc)) {
			fprintf(stderr, "Key corrupted!\n");
			return 16;
		}
	}

	fclose(ko);
	return 0;
}

static int sign_file(const char *msgfile, const struct ecc_key *mkey,
		const char *sigfile, const char *sdname)
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

	ecc_sign(sig, mkey, (unsigned char *)mesg, mstat.st_size, sdname);

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
	if (!crc32_check((unsigned char *)sig, sizeof(struct ecc_sig), crc)) {
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
	int fin, opt, action, retv, fnamlen;
	const char *keyfile, *msgfile, *keystr;
	char *sigfile, *exbuf;
	const char *sdname;
	extern int optind, opterr, optopt;
	extern char *optarg;

	retv = 0;
	keyfile = NULL;
	msgfile = NULL;
	sigfile = NULL;
	keystr = NULL;
	opterr = 0;
	fin = 0;
	action = 0;
	sdname = NULL;
	do {
		opt = getopt(argc, argv, ":a:k:svgei:");
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
		case 'a':
			sdname = optarg;
			break;
		case 'e':
			action |= EXPORT_KEY;
			break;
		case 'i':
			action |= IMPORT_KEY;
			keystr = optarg;
			break;
		case 'k':
			keyfile = optarg;
			break;
		case 's':
			action |= SIGN_FILE;
			break;
		case 'v':
			action |= SIG_VERIFY;
			break;
		case 'g':
			action |= GEN_KEY;
			break;
		default:
			assert(0);
		}
	} while (fin == 0);
	if (!keyfile) {
		fprintf(stderr, "A key file must be supplied!\n");
		return 20;
	}
	if (!sdname)
		sdname = "hw:0,0";

	msgfile = argv[optind];
	if (!msgfile && (action & (SIG_VERIFY|SIGN_FILE))) {
		fprintf(stderr, "Usage: %s -k keyfile [-g] [-s|-v]"
				" file [file.sig]\n", argv[0]);
		return 10;
	}
	fnamlen = 0;
	if (msgfile)
		fnamlen = strlen(msgfile);
	buffer = malloc(sizeof(struct ecc_key)+fnamlen+5);
	if (!buffer) {
		fprintf(stderr, "Out of Memory!\n");
		return 10000;
	}
	if (optind+1 < argc)
		sigfile = argv[optind+1];
	else {
		sigfile = buffer + sizeof(struct ecc_key);
		if (fnamlen > 0) {
			strcpy(sigfile, msgfile);
			strcat(sigfile, ".sig");
		}
	}

	mkey = buffer;
	ecc_init();

	key_process(mkey, keyfile, action, keystr, sdname);

	if (action & SIGN_FILE)
		sign_file(msgfile, mkey, sigfile, sdname);

	if (action & SIG_VERIFY)
		if (!verify_file(msgfile, mkey, sigfile))
			fprintf(stderr, "Signuature verification failed!\n");

	if (action & EXPORT_KEY) {
		exbuf = malloc(256);
		ecc_key_export(exbuf, 256, mkey, ECCKEY_PUB|ECCKEY_BRIEF);
		printf("Pub: %s\n", exbuf);
		free(exbuf);
	}

	free(buffer);
	return retv;
}
