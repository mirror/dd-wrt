#include "Memory.h"

#include <string.h>

uint32_t Ham_Swap32(const uint32_t value)
{
	uint32_t swapped;
	((uint8_t *)&swapped)[0] = ((uint8_t *)&value)[3];
	((uint8_t *)&swapped)[1] = ((uint8_t *)&value)[2];
	((uint8_t *)&swapped)[2] = ((uint8_t *)&value)[1];
	((uint8_t *)&swapped)[3] = ((uint8_t *)&value)[0];
	return swapped;
}

void Ham_WriteAndSeek(void **dst, const void *src, const size_t size)
{
	if (!dst || !*dst)
	{
		return;
	}

	uint8_t **buf = (uint8_t **)dst;

	memcpy(*buf, src, size);
	*buf += size;
}
