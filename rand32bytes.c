#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "loglog.h"

int rand32bytes(unsigned char *rndbuf, int len, int strong)
{
	const char *rnddev = "/dev/urandom";
	int numbytes = 0;
	FILE *rndh;

	if (strong)
		rnddev = "/dev/random";
	rndh = fopen(rnddev, "rb");
	if (!rndh) {
		logmsg(LOG_ERR, "Cannot open %s: %s\n", rnddev,
				strerror(errno));
		return numbytes;
	}
	numbytes = fread(rndbuf, 1, len, rndh);
	fclose(rndh);
	return numbytes;
}
