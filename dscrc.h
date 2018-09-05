/*
 * CRC8, CRC16 and CRC32 algothim implementation, Dashi Cao dscao999@hotmail.com caods1@lenovo.com
 *
 */
#ifndef CRC_DSCAO__
#define CRC_DSCAO__
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

unsigned int crc32(const unsigned char *bytes, int len);

static inline
int crc32_check(const unsigned char *bytes, int len, unsigned int crc)
{
        unsigned int ncrc;

        ncrc = crc32(bytes, len);
        return ncrc == crc;
}
#endif  /* CRC_DSCAO__ */
