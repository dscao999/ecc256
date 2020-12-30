#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "md5.h"

int main(int argc, char *argv[])
{
	struct md5_ctx md5;
	wd8 digest[16], buf[256];
	int i, numb, optdone, md5_file, opt;
	const char *str;
	FILE *fin;
	extern int optopt, optind;

	md5_file = 0;
	optdone = 0;
	do {
		opt = getopt(argc, argv, "f");
		switch(opt) {
		case '?':
			fprintf(stderr, "Unknown option: %c ignored\n", optopt);
			break;
		case 'f':
			md5_file = 1;
			break;
		case -1:
			optdone = 1;
			break;
		}
	} while (optdone == 0);

	if (argc == optind) {
		printf("Usage: %s [-f] string/file-name\n", argv[0]);
		return 1;
	}
	str = argv[optind];

	md5_init(&md5);

	if (!md5_file) {
		md5_update(&md5, (unsigned char *)str, strlen(str));
	} else {
		fin = fopen(str, "rb");
		if (!fin) {
			fprintf(stderr, "Cannot open file '%s': %s\n", str,
					strerror(errno));
			return 2;
		}
		do {
			numb = fread(buf, 1, 256, fin);
			md5_update(&md5, (const wd8 *)buf, numb);
		} while (!feof(fin));
		fclose(fin);
	}

	md5_exit(digest, &md5);
	printf("MD5: %s\n", str);
	for (i = 0; i < 16; i++)
		printf("%02X", (unsigned int)digest[i]);
	printf("\n");
	return 0;
}
