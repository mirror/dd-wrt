#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

#ifdef BYTE_ORDER_BIG_ENDIAN
#define BigEndian32
#else
#define BigEndian32 Ham_Swap32
#endif

#define CompressionBufferSize(original_size) (original_size * 2 + 256)

uint32_t Ham_Swap32(const uint32_t value);

void Ham_WriteAndSeek(void **dst, const void *src, const size_t size);

#endif
