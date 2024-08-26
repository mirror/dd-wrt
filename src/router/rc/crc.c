
#include "typedefs.h"
#include "crc.h"

static uint32 crc32(void *vdata, int size, uint32 crc)
{
	unsigned char *data = (unsigned char *)vdata;
	unsigned char *end = data + size;
	int i;

	while (data < end) {
		crc ^= *data++;

		for (i = 0; i < 8; i++) {
			uint t = ~((crc & 1) - 1);
			crc = (crc >> 1) ^ (0xEDB88320 & t);
		}
	}

	return ~crc;
}
