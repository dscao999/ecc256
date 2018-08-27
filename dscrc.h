#ifndef CRC_DSCAO__
#define CRC_DSCAO__
unsigned int crc16(const unsigned char *bytes, int len);

static inline
int crc16_confirm(const unsigned char *bytes, int len, unsigned int crc)
{
	unsigned int ncrc;

	ncrc = crc16(bytes, len);
	return ncrc == crc;
}

unsigned int crc32(const unsigned char *bytes, int len);

static inline
int crc32_confirm(const unsigned char *bytes, int len, unsigned int crc)
{
        unsigned int ncrc;

        ncrc = crc32(bytes, len);
        return ncrc == crc;
}
#endif  /* CRC_DSCAO__ */
