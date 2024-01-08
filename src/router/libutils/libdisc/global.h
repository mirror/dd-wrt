#include <disc.h>
#include <endian.h>

#ifndef bswap_64
#define bswap_64(x)                                                                                                              \
	((((x) & 0x00000000000000FFULL) << 56) | (((x) & 0x000000000000FF00ULL) << 40) | (((x) & 0x0000000000FF0000ULL) << 24) | \
	 (((x) & 0x00000000FF000000ULL) << 8) | (((x) & 0x000000FF00000000ULL) >> 8) | (((x) & 0x0000FF0000000000ULL) >> 24) |   \
	 (((x) & 0x00FF000000000000ULL) >> 40) | (((x) & 0xFF00000000000000ULL) >> 56))
#endif

#ifndef bswap_32
#define bswap_32(x)                                                                              \
	((uint32_t)((((uint32_t)(x) & 0x000000FF) << 24) | (((uint32_t)(x) & 0x0000FF00) << 8) | \
		    (((uint32_t)(x) & 0x00FF0000) >> 8) | (((uint32_t)(x) & 0xFF000000) >> 24)))
#endif

#ifndef htobe16
#if !defined(WORDS_BIGENDIAN)
#define htobe16(x) bswap_16(x)
#define htole16(x) (x)
#define be16toh(x) bswap_16(x)
#define le16toh(x) (x)
#define htobe32(x) bswap_32(x)
#define htole32(x) (x)
#define be32toh(x) bswap_32(x)
#define le32toh(x) (x)
#define htobe64(x) bswap_64(x)
#define htole64(x) (x)
#define be64toh(x) bswap_64(x)
#define le64toh(x) (x)
#else
#define htobe16(x) (x)
#define htole16(x) bswap_16(x)
#define be16toh(x) (x)
#define le16toh(x) bswap_16(x)
#define htobe32(x) (x)
#define htole32(x) bswap_32(x)
#define be32toh(x) (x)
#define le32toh(x) bswap_32(x)
#define htobe64(x) (x)
#define htole64(x) bswap_64(x)
#define be64toh(x) (x)
#define le64toh(x) bswap_64(x)
#endif
#endif

#define be32_to_cpu(x) ((uint32_t)be32toh(x))
#define be64_to_cpu(x) ((uint64_t)be64toh(x))

void bailout(const char *msg, ...) __attribute__((noreturn));

void bailoute(const char *msg, ...) __attribute__((noreturn));
