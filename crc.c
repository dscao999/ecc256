#include <stdio.h>
#include <stdlib.h>
#include "dscrc.h"

int main(int argc, char *argv[])
{
	int i, len;
	unsigned int num, crc;
	unsigned char *dat;

	dat = malloc(1024);
	printf("Input: ");
	fflush(stdout);
	len = 0;
	do {
		scanf("%x", &num);
		if (num > 255)
			break;
		dat[len++] = num;
	} while (1);
	printf("Length: %d-->", len);
	for (i = 0; i < len; i++) {
		printf("%02X ", (unsigned int)dat[i]);
		if (((i+1) & 0x0f) == 0)
			printf("\n");
	}
	if ((len & 0x0f) != 0)
		printf("\n");

	crc = crc8(dat, len);
	printf("CRC8: %02X\n", crc);
	dat[len] = crc & 0x0ff;
	crc = crc8(dat, len+1);
	printf("CRC8: %02X\n", crc);
	printf("\n");

	crc = crc16(dat, len);
	printf("CRC16: %04X\n", crc);
	dat[len] = (crc >> 8) & 0x0ff;
	dat[len+1] = crc & 0x0ff;
	crc = crc16(dat, len+2);
	printf("CRC16: %04X\n", crc);
	printf("\n");

	crc = crc32(dat, len);
	printf("CRC32: %08X\n", crc);
	dat[len] = crc >> 24;
	dat[len+1] = (crc >> 16) & 0x0ff;
	dat[len+2] = (crc >> 8) & 0x0ff;
	dat[len+3] = crc & 0x0ff;
	crc = crc32(dat, len+4);
	printf("CRC32: %08X\n", crc);
	printf("\n");

	free(dat);
	return 0;
}
