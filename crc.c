#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "dscrc.h"

int main(int argc, char *argv[])
{
	int len;
	unsigned int crc;
	unsigned char *dat;
	char *ln;

	dat = malloc(1024);
	printf("Input: ");
	fflush(stdout);
	len = 0;
	do {
		ln = readline("? ");
		len = strlen(ln);
		if (len == 0)
			break;

		memcpy(dat, ln, len);
		crc = crc8(dat, len);
		printf("CRC8: %02X\n", crc);
		dat[len] = crc & 0x0ff;
		crc = crc8(dat, len+1);
		printf("CRC8: %02X\n", crc);

		crc = crc16(dat, len);
		printf("CRC16: %04X\n", crc);
		dat[len] = (crc >> 8) & 0x0ff;
		dat[len+1] = crc & 0x0ff;
		crc = crc16(dat, len+2);
		printf("CRC16: %04X\n", crc);

		crc = crc32(dat, len);
		printf("CRC32: %08X\n", crc);
		dat[len] = crc >> 24;
		dat[len+1] = (crc >> 16) & 0x0ff;
		dat[len+2] = (crc >> 8) & 0x0ff;
		dat[len+3] = crc & 0x0ff;
		crc = crc32(dat, len+4);
		printf("CRC32: %08X\n", crc);
	} while (1);
	free(dat);
	return 0;
}
