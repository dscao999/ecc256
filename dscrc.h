/*
 * CRC8, CRC16 and CRC32 algothim implementation, Dashi Cao dscao999@hotmail.com caods1@lenovo.com
 *
 */
#ifndef CRC_DSCAO__
#define CRC_DSCAO__
#include "miscs.h"

unsigned int crc8(const unsigned char *bytes, int len);
static inline
int crc8_check(const unsigned char *bytes, int len, unsigned int crc)
{
	unsigned int ncrc;
	ncrc = crc8(bytes, len);
	return ncrc == crc;
}

unsigned int crc16(const unsigned char *bytes, int len);
static inline
int crc16_check(const unsigned char *bytes, int len, unsigned int crc)
{
	unsigned int ncrc;

	ncrc = crc16(bytes, len);
	return ncrc == crc;
}

__declspec(dllexport) unsigned int __cdecl crc32(const unsigned char *bytes, int len);
__declspec(dllexport) int __cdecl crc32_check(const unsigned char* bytes, int len, unsigned int crc);
#endif  /* CRC_DSCAO__ */
