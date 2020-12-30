#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "md5.h"

int main(int argc, char *argv[])
{
	struct md5_ctx md5;
	wd8 digest[16], buf[256];
	int i, numb;
	const char *fname;
	FILE *fin;

	if (argc < 2) {
		printf("Usage: %s string/file-name\n", argv[0]);
		return 1;
	}
	md5_init(&md5);

	fname = argv[1];
	fin = fopen(fname, "rb");
	if (fin == NULL) {
		md5_update(&md5, (unsigned char *)fname, strlen(fname));
	} else {
		do {
			numb = fread(buf, 1, 256, fin);
			md5_update(&md5, (const wd8 *)buf, numb);
		} while (!feof(fin));
	}
	md5_exit(digest, &md5);
	printf("MD5: %s\n", argv[1]);
	for (i = 0; i < 16; i++)
		printf("%02X", (unsigned int)digest[i]);
	printf("\n");
}
