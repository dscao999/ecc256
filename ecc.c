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
#include "base64.h"
#include "alsarec.h"

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
#define HASH_PUBKEY	0x20

struct keyparam {
	struct ecc_key key;
	const char *keyfile;
	const char *sdname;
	const char *pass;
	const char *keystr;
	int nosigfile;
	int exstr;
};

static int key_save2file(const struct keyparam *param)
{
	FILE *ko;
	int retv = 0, plen;

	if (!param->keyfile) {
		logmsg(LOG_ERR, "Cannot save key to file, no file specified.");
		return 3;
	}
	ko = fopen(param->keyfile, "wb");
	if (!check_pointer(ko, LOG_ERR, "Cannot open file %s for writing.\n",
				param->keyfile))
		return 4;
	if (param->pass)
		plen = strlen(param->pass);
	else
		plen = 0;
	retv = ecc_writkey(&param->key, ko, param->pass, plen);
	fclose(ko);
	return retv;
}

static int key_process(struct keyparam *param, int action)
{
	FILE *ko;
	int plen = 0, retv = 0;

	if (param->pass)
		plen = strlen(param->pass);
	if (action & GEN_KEY) {
		ecc_genkey(&param->key, 5);
		key_save2file(param);
	} else if (action & IMPORT_KEY) {
		retv = ecc_key_import(&param->key, param->keystr);
		if (retv == -65)
			logmsg(LOG_ERR, "Invalid public key!\n");
		else if (!ecc_pubkey_only(&param->key))
			key_save2file(param);
		if (retv < 0)
			logmsg(LOG_WARNING, "Big Number Invlid/Overflow.\n");
	} else if (param->keyfile) {
		ko = fopen(param->keyfile, "rb");
		if (check_pointer(ko, LOG_ERR, "Cannot open key file %s.",
					param->keyfile)) {
			retv = ecc_readkey(&param->key, ko, param->pass, plen);
			fclose(ko);
		}
	} else
		logmsg(LOG_ERR, "A key file must be specified.\n");
	return retv;
}

static int sign_file(const struct keyparam *param,
		const char *msgfile, char *sigfile)
{
	FILE *mi;
	void *area;
	char *mesg;
	struct stat mstat;
	struct ecc_sig *sig;
	unsigned int crc;
	int sysret, retv, len, rlen, slen;

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

	ecc_sign(sig, &param->key, (unsigned char *)mesg, mstat.st_size);

	if (param->nosigfile == 0) {
		mi = fopen(sigfile, "wb");
		if (!mi) {
			logmsg(LOG_ERR, "Cannot open file %s for writing.\n",
					sigfile);
			retv = errno;
			goto exit_20;
		}
		sysret = fwrite(sig, sizeof(struct ecc_sig), 1, mi);
		if (sysret != 1)
			logmsg(LOG_ERR, "Write error %s: %s\n", sigfile,
					strerror(errno));
		crc = crc32((unsigned char *)sig, sizeof(struct ecc_sig));
		sysret = fwrite(&crc, sizeof(crc), 1, mi);
	} else {
		rlen = bin2str_b64(sigfile, 70, (const unsigned char *)sig->sig_r, ECCKEY_INT_LEN*4);
		sigfile[rlen] = ',';
		slen = bin2str_b64(sigfile+rlen+1, 69,
				(const unsigned char *)sig->sig_s, ECCKEY_INT_LEN*4);
		assert(rlen+slen+1 < 140);
	}

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

	if (param->nosigfile == 0) {
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
			logmsg(LOG_ERR, "Cannot open file %s for reading.\n",
					sigfile);
			retv = errno;
			goto exit_20;
		}
		fread(sig, sizeof(struct ecc_sig), 1, mi);
		fread(&crc, sizeof(crc), 1, mi);
		if (!crc32_check((unsigned char *)sig, sizeof(struct ecc_sig),
					crc)) {
			logmsg(LOG_ERR, "Corrupted signature!\n");
			goto exit_20;
		}
		if (param->exstr) {
			char *xbuf = malloc(100);
			ecc_sig2str(xbuf, 100, sig);
			printf("%s\n", xbuf);
		}
		retv = ecc_verify(sig, &param->key, mesg, len);
	} else {
		if (ecc_str2sig(sig, sigfile) < 0) {
			logmsg(LOG_ERR, "Overflow, not a valid signiture.\n");
			goto exit_20;
		}
		retv = ecc_verify(sig, &param->key, mesg, len);
	}

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
	int flag;
	const char *msgfile, *fbname;
	char *sigfile, *exbuf;
	extern int optind, opterr, optopt;
	extern char *optarg;

	alsa_init();
	kparam = malloc(sizeof(struct keyparam));
	if (!check_pointer(kparam, LOG_CRIT, nomem))
		return NOMEM;
	kparam->pass= NULL;
	kparam->keyfile = NULL;
	kparam->keystr = NULL;
	kparam->sdname = NULL;
	kparam->nosigfile = 0;
	kparam->exstr = 0;
	sigfile = NULL;
	msgfile = NULL;
	retv = 0;
	opterr = 0;
	fin = 0;
	action = 0;
	do {
		opt = getopt(argc, argv, ":a:hk:s::v::ge::i:p:x");
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
		case 'x':
			kparam->exstr = 1;
			break;
		case 'h':
			action |= HASH_PUBKEY;
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
			if (optarg)
				kparam->nosigfile = 1;
			break;
		case 'v':
			action |= SIG_VERIFY;
			if (optarg)
				kparam->nosigfile = 1;
			break;
		case 'g':
			action |= GEN_KEY;
			break;
		default:
			assert(0);
		}
	} while (fin == 0);
	if (!kparam->sdname)
		kparam->sdname = "hw:0,0";

	if (optind < argc)
		msgfile = argv[optind];
	if (!msgfile && (action & (SIG_VERIFY|SIGN_FILE))) {
		fprintf(stderr, "Usage: %s -k keyfile [-g] [-s|-v]"
				" file [file.sig]\n", argv[0]);
		return 10;
	}
	fnamlen = 0;
	if (msgfile)
		fnamlen = strlen(msgfile);

	buffer = malloc(fnamlen+600);
	if (!check_pointer(buffer, LOG_CRIT, nomem)) {
		free(kparam);
		return NOMEM;
	}
	if (action & (SIG_VERIFY|SIGN_FILE)) {
		sigfile = buffer;
		*sigfile = 0;
		if (optind + 1 < argc)
			sigfile = argv[optind+1];
		else if (kparam->nosigfile == 0) {
			fbname = strrchr(msgfile, '/');
			if (fbname)
				strcpy(sigfile, fbname+1);
			else
				strcpy(sigfile, msgfile);
			strcat(sigfile, ".sig");
		}
	}
	exbuf = buffer + 300;

	ecc_init();

	if (key_process(kparam, action) < 0)
		return 1;

	if (action & SIGN_FILE) {
		if (!ecc_pubkey_only(&kparam->key))
			sign_file(kparam, msgfile, sigfile);
		else
			logmsg(LOG_ERR, "Cannot sign without private key.\n");
	}

	if (action & SIG_VERIFY)
		if (!verify_file(kparam, msgfile, sigfile))
			logmsg(LOG_ERR, "Signuature verification failed!\n");

	if (action & EXPORT_KEY) {
		flag = ECCKEY_EXPUB;
		if (action & EXPORT_PRIV)
			flag = ECCKEY_EXPRIV;
		ecc_key_export(exbuf, 256, &kparam->key, flag);
		printf("%s\n", exbuf);
	}
	if (action & HASH_PUBKEY) {
		ecc_key_hash(exbuf, 256, &kparam->key);
		printf("%s\n", exbuf);
	}
	if (kparam->nosigfile && (action & SIGN_FILE))
		printf("%s\n", sigfile);

	free(buffer);
	free(kparam);
	alsa_exit();
	return retv;
}
