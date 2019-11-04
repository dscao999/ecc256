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
#include "loglog.h"

static inline int malloc_len(int len)
{
	return ((((len - 1) >> 4) + 1) << 4);
}

#define GEN_KEY		0x01
#define SIGN_FILE	0x02
#define SIG_VERIFY	0x04
#define IMPORT_KEY	0x40
#define EXPORT_KEY	0x80
#define EXPORT_PRIV	0x10

struct keyparam {
	struct ecc_key key;
	const char *keyfile;
	const char *sdname;
	const char *pass;
	const char *keystr;
};

static int key_process(struct keyparam *param, int action)
{
	FILE *ko;
	int plen = 0;
	int retv = 0;

	if ((action & GEN_KEY) || (action & IMPORT_KEY))
		ko = fopen(param->keyfile, "wb");
	else
		ko = fopen(param->keyfile, "rb");
	if (!ko) {
		fprintf(stderr, "Cannot open key file %s for read/write!\n",
				param->keyfile);
		return 4;
	}
	if (param->pass)
		plen = strlen(param->pass);
	if (action & GEN_KEY) {
		ecc_genkey(&param->key, 5, param->sdname);
		ecc_writkey(&param->key, ko, param->pass, plen);
	} else if (action & IMPORT_KEY) {
		ecc_key_import(&param->key, param->keystr);
		ecc_writkey(&param->key, ko, param->pass, plen);
	} else {
		retv = ecc_readkey(&param->key, ko, param->pass, plen);
	}

	fclose(ko);
	return retv;
}

static int sign_file(const struct keyparam *param,
		const char *msgfile, const char *sigfile)
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
		return errno;
	}
	len = malloc_len(mstat.st_size);
	area = malloc(len+sizeof(struct ecc_sig));
	if (!check_pointer(area, LOG_CRIT, nomem))
		return NOMEM;
	mesg = area;
	sig =  area + len;

	mi = fopen(msgfile, "rb");
	if (!mi) {
		logmsg(LOG_ERR, "Cannot openfile %s for reading.\n", msgfile);
		retv = errno;
		goto exit_10;
	}
	sysret = fread(mesg, 1, mstat.st_size, mi);
	if (sysret != mstat.st_size) {
		logmsg(LOG_ERR, "Read file %s error\n", msgfile);
		retv = 12;
		goto exit_20;
	}
	fclose(mi);
	mi = NULL;

	ecc_sign(sig, &param->key, (unsigned char *)mesg, mstat.st_size,
			param->sdname);

	mi = fopen(sigfile, "wb");
	if (!mi) {
		logmsg(LOG_ERR, "Cannot open file %s for writing.\n", sigfile);
		retv = errno;
		goto exit_20;
	}
	sysret = fwrite(sig, sizeof(struct ecc_sig), 1, mi);
	if (sysret != 1)
		logmsg(LOG_ERR, "Write error %s: %s\n", sigfile,
				strerror(errno));
	crc = crc32((unsigned char *)sig, sizeof(struct ecc_sig));
	sysret = fwrite(&crc, sizeof(crc), 1, mi);

exit_20:
	if (mi)
		fclose(mi);
exit_10:
	free(area);
	return retv;
}

static int verify_file(const struct keyparam *param, 
		const char *msgfile, const char *sigfile)
{
	FILE *mi;
	int retv, sysret;
	struct stat fst;
	void *area = NULL;
	unsigned char *mesg;
	struct ecc_sig *sig;
	unsigned int crc;
	unsigned long len;

	retv = 0;
	sysret = stat(msgfile, &fst);
	if (sysret == -1) {
		logmsg(LOG_ERR, "File %s error: %s\n", msgfile,
			strerror(errno));
		return errno;
	}
	mi = fopen(msgfile, "rb");
	if (!mi) {
		logmsg(LOG_ERR, "Cannot open file %s for read.\n", msgfile);
		return errno;
	}
	area = malloc(malloc_len(fst.st_size)+sizeof(struct ecc_sig));
	if (!check_pointer(area, LOG_CRIT, nomem)) {
		retv = NOMEM;
		goto exit_10;
	}
	len = fst.st_size;
	sig = area + malloc_len(fst.st_size);
	mesg = area;
	sysret = fread(mesg, 1, fst.st_size, mi);
	if (sysret != fst.st_size) {
		logmsg(LOG_ERR, "File read error!\n");
		goto exit_20;
	}
	fclose(mi);
	mi = NULL;

	sysret = stat(sigfile, &fst);
	if (sysret == -1) {
		logmsg(LOG_ERR, "File %s error: %s\n", sigfile,
			strerror(errno));
		retv = errno;
		goto exit_20;
	}
	if (fst.st_size != 68) {
		fprintf(stderr, "Invalid Signature %s\n", sigfile);
		goto exit_20;
	}
	mi = fopen(sigfile, "rb");
	if (!mi) {
		logmsg(LOG_ERR, "Cannot open file %s for reading.\n", sigfile);
		retv = errno;
		goto exit_20;
	}
	fread(sig, sizeof(struct ecc_sig), 1, mi);
	fread(&crc, sizeof(crc), 1, mi);
	if (!crc32_check((unsigned char *)sig, sizeof(struct ecc_sig), crc)) {
		logmsg(LOG_ERR, "Corrupted signature!\n");
		goto exit_20;
	}
	retv = ecc_verify(sig, &param->key, mesg, len);

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
	struct keyparam *kparam;
	void *buffer;
	int fin, opt, action, retv, fnamlen;
	const char *msgfile, *fbname;
	char *sigfile, *exbuf;
	extern int optind, opterr, optopt;
	extern char *optarg;

	kparam = malloc(sizeof(struct keyparam));
	if (!check_pointer(kparam, LOG_CRIT, nomem))
		return NOMEM;
	kparam->pass= NULL;
	kparam->keyfile = NULL;
	kparam->keystr = NULL;
	kparam->sdname = NULL;
	sigfile = NULL;
	retv = 0;
	opterr = 0;
	fin = 0;
	action = 0;
	do {
		opt = getopt(argc, argv, ":a:k:svge::i:p:");
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
		case 'p':
			kparam->pass= optarg;
			break;
		case 'a':
			kparam->sdname = optarg;
			break;
		case 'e':
			action |= EXPORT_KEY;
			if (optarg)
				action |= EXPORT_PRIV;
			break;
		case 'i':
			action |= IMPORT_KEY;
			kparam->keystr = optarg;
			break;
		case 'k':
			kparam->keyfile = optarg;
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
	if (!kparam->keyfile) {
		fprintf(stderr, "A key file must be supplied!\n");
		return 20;
	}
	if (!kparam->sdname)
		kparam->sdname = "hw:0,0";

	msgfile = argv[optind];
	if (!msgfile && (action & (SIG_VERIFY|SIGN_FILE))) {
		fprintf(stderr, "Usage: %s -k keyfile [-g] [-s|-v]"
				" file [file.sig]\n", argv[0]);
		return 10;
	}
	fnamlen = 0;
	if (msgfile)
		fnamlen = strlen(msgfile);
	buffer = malloc(fnamlen+512);
	if (!check_pointer(buffer, LOG_CRIT, nomem)) {
		free(kparam);
		return NOMEM;
	}
	if (optind+1 < argc)
		sigfile = argv[optind+1];
	else {
		sigfile = buffer;
		if (fnamlen > 0) {
			fbname = strrchr(msgfile, '/');
			if (fbname)
				strcpy(sigfile, fbname);
			else
				strcpy(sigfile, msgfile);
			strcat(sigfile, ".sig");
			exbuf = buffer + fnamlen + 8;
		} else
			exbuf = buffer;
	}

	ecc_init();

	if (key_process(kparam, action))
		return 1;

	if (action & SIGN_FILE)
		sign_file(kparam, msgfile, sigfile);

	if (action & SIG_VERIFY)
		if (!verify_file(kparam, msgfile, sigfile))
			fprintf(stderr, "Signuature verification failed!\n");

	if (action & EXPORT_KEY) {
		if (action & EXPORT_PRIV)
			ecc_key_export(exbuf, 256, &kparam->key, ECCKEY_EXPRIV);
		else
			ecc_key_export(exbuf, 256, &kparam->key, ECCKEY_EXPUB);
		printf("%s\n", exbuf);
	}

	free(buffer);
	free(kparam);
	return retv;
}
